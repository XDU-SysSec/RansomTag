#!/usr/bin/python
# -*- coding: UTF-8 -*-

import argparse
import textwrap
import os
import sys
import subprocess
import re
import struct
import mmap
import pdb
import ctypes
import directio
import copy

versions = 5   # multi-versions numbers of file records
inode_number_per_group = 50   # set a small range of inode numbers

def trigger_mfttag_start(fs, block_size):
    #for i in range(3):
    read_blocks(fs, block_size, 120000, 1)

def trigger_mfttag_end(fs, block_size):
    #for i in range(3):
    read_blocks(fs, block_size, 120001, 1)


def trigger_recovertag_start(fs, block_size):
    #for i in range(3):
    read_blocks(fs, block_size, 120002, 1)

def trigger_recovertag_end(fs, block_size):
    #for i in range(3):
    read_blocks(fs, block_size, 120003, 1)

# make from 5-9 list of integers
def split_range(numstr):
    num1, num2 = numstr.split('-')
    return list(range(int(num1), int(num2)+1))

# join continuous data in array -> [1, 2, 4, 5] --> ({first:1, total:2}, {first:4, total:2})
def join_ranges(arr):
    last_from = None
    last_to = None

    data = []
    for entry in arr:
        if last_to is None:
            last_from = entry
            last_to = entry
            continue

        if entry == last_to + 1:
            last_to = entry
            continue

        # commit
        data.append({
            'first': last_from,
            'total': last_to-last_from+1
        })
        last_from = entry
        last_to = entry

    if last_from is None or last_to is None:
        return []

    data.append({
        'first': last_from,
        'total': last_to-last_from+1
    })
    return data

# read superblock
def get_super(fs):
    process = subprocess.Popen(
        ['dumpe2fs', fs],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )
    stdout, stderr = process.communicate()

    data = stdout.decode("utf-8")
    if process.returncode != 0:
        raise Exception(data)

    sb, blocks = data.split('\n\n')

    data = {}

    rows = sb.strip().split('\n')
    for row in rows:
        # prop:  value
        prop, val = re.compile(":\s+").split(row)

        # parse integers
        if re.match("^-?\d+$", val):
            data[prop] = int(val)
        else:
            data[prop] = val

    # split
    data['Filesystem features'] = data['Filesystem features'].split(' ')

    data['Blocks Groups Flags'] = []
    rows = blocks.strip().split('\n')
    for row in rows:
        # must start with group
        match = re.match("^Group.*\[(.*?)\]$", row)
        if not match:
            continue

        flags = match.group(1).split(", ")
        data['Blocks Groups Flags'].append(flags)

    return data

# test if is flag in block group
def test_block_groups_flag(sb, flag_name, group_index):
    return group_index in sb['Blocks Groups Flags'] and flag_name in sb['Blocks Groups Flags'][group_index]

# get infos about block groups
def get_block_groups(fs):
    process = subprocess.Popen(
        ['dumpe2fs', '-g', fs],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )
    stdout, stderr = process.communicate()

    data = stdout.decode("utf-8").strip()
    if process.returncode != 0:
        raise Exception(data)

    rows = data.split('\n')

    # remove header
    rows.pop(0)

    # group:block:super:gdt:bbitmap:ibitmap:itable

    data = []
    for row in rows:
        col = row.split(':')

        data.append({
            'group': int(col[0]),
            'block': int(col[1]),
            'super': None if col[2] == '-1' else int(col[2]),
            'gdt': None if col[3] == '-1' else split_range(col[3]),
            'bbitmap': int(col[4]),
            'ibitmap': int(col[5]),
            'itable': int(col[6])
        })

    return data


# read any given number of blocks from fs
def read_blocks(fs, block_size, addr, total=1):
    #pdb.set_trace()
    fd = os.open(fs, os.O_RDONLY | os.O_DIRECT)  # O_DIRECT is required for avoiding reading kernel cache iteratively.
    m = directio.pread(fd, total * block_size, addr * block_size)
    os.close(fd)
    return m

    '''
    with open(fs, "rb") as fh:
        fh.seek(addr * block_size)
        return fh.read(total * block_size)
    '''
    
# get one bit from byte array
def access_bit(data, num):
    base = int(num // 8)
    shift = int(num % 8)
    return (data[base] & (1<<shift)) >> shift

# get array of indexes, that are set to 1 in bitmp. If total_items is set, only indexes < total_items are counted.
def parse_bitmap(data, total_items=None):
    indexes = []
    for i in range(len(data)*8):
        if total_items is not None and i >= total_items:
            break
        if access_bit(data, i):
            indexes.append(i)
    return indexes

# [ 'inode_id': 'physical_addr', ... ]
def get_inodes_addresses(fs, sb=None):
    # get blocks
    bgs = get_block_groups(fs)

    # get inodes with their physical address
    inodes = {}
    for bg in bgs:
        # only initialized inodes
        if test_block_groups_flag(sb, 'INODE_UNINIT', bg['group']):
            continue

        # get & parse bitmap
        bitmap = read_blocks(fs, sb['Block size'], bg['ibitmap'])
        indexes = parse_bitmap(bitmap, sb['Inodes per group'])

        # get base address for inode table in this bg
        base_address = bg['itable'] * sb['Block size']

        # loop through used inodes in this bg
        for index in indexes:
            inode_id = index + (sb['Inodes per group'] * bg['group']) + 1
            raw_address = (index * sb['Inode size']) + base_address

            inodes[inode_id] = raw_address

    return inodes


#
# INODE
#

# get filetype from `i_mode`
def get_filetype(i_mode):
    filetype_flag = (i_mode & 0xF000)

    if filetype_flag == 0x1000: # S_IFIFO (FIFO)
        return 'S_IFIFO'
    if filetype_flag == 0x2000: # S_IFCHR (Character device)
        return 'S_IFCHR'
    if filetype_flag == 0x4000: # S_IFDIR (Directory)
        return 'S_IFDIR'
    if filetype_flag == 0x6000: # S_IFBLK (Block device)
        return 'S_IFBLK'
    if filetype_flag == 0x8000: # S_IFREG (Regular file)
        return 'S_IFREG'
    if filetype_flag == 0xA000: # S_IFLNK (Symbolic link)
        return 'S_IFLNK'
    if filetype_flag == 0xC000: # S_IFSOCK (Socket)
        return 'S_IFSOCK'

# get only relevant flags from `i_flags`
def get_flags(i_flags):
    flags = []

    # Directory has hashed indexes
    if i_flags & 0x1000:
        flags.append('EXT4_INDEX_FL')
    # This is a huge file
    if i_flags & 0x40000:
        flags.append('EXT4_HUGE_FILE_FL')
    # Inode uses extents
    if i_flags & 0x80000:
        flags.append('EXT4_EXTENTS_FL')
    # Inode has inline data
    if i_flags & 0x10000000:
        flags.append('EXT4_INLINE_DATA_FL')

    return flags

# join int32 with (int32, int16 or int8)
def join_int32(hi, lo):
    # join hi + lo
    num = (hi << 32) +  lo

    # test
    src_bits = "{0:032b}".format(hi) + "{0:032b}".format(lo)
    dst_bits = "{0:064b}".format(num)
    assert src_bits == dst_bits

    return num

# get relevant data from inode
def inode_parse(data):
    # 0x00   __le16   i_mode
    i_mode = struct.unpack('<H', data[0:2])[0]
    # 0x04   __le32   i_size_lo
    i_size_lo = struct.unpack('<I', data[4:8])[0]
    # 0x1A   __le16   i_links_count
    i_links_count = struct.unpack('<H', data[26:26+2])[0]
    # 0x20   __le32   i_flags
    i_flags = struct.unpack('<I', data[32:32+4])[0]
    # 0x28  60 bytes  i_block
    i_block = data[40:100]
    # 0x6C   __le32   i_size_high
    i_size_high = struct.unpack('<I', data[108:112])[0]

    return {
        'filetype': get_filetype(i_mode),
        'flags': get_flags(i_flags),
        'links_count': i_links_count,
        'data': i_block,
        'size': join_int32(i_size_high, i_size_lo)
    }

#
# EXTENT TREE STRUCTS
#
def ext4_extent_header(data):
    assert len(data) >= 12

    res = {}
    # 0x00   __le16   eh_magic
    eh_magic = struct.unpack('<H', data[0:2])[0]
    # 0x02   __le16   eh_entries
    res['eh_entries'] = struct.unpack('<H', data[2:4])[0]
    # 0x04   __le16   eh_max
    res['eh_max'] = struct.unpack('<H', data[4:6])[0]
    # 0x06   __le16   eh_depth
    res['eh_depth'] = struct.unpack('<H', data[6:8])[0]
    # 0x08   __le32   eh_generation
    #res['eh_generation'] = struct.unpack('<I', data[8:12])[0] # not standard ext4

    # expect magic to be set
    assert eh_magic == 0xF30A
    return res

def ext4_extent_idx(data):
    assert len(data) >= 12

    res = {}
    # 0x00   __le32   ei_block
    res['ei_block'] = struct.unpack('<I', data[0:4])[0]
    # 0x04   __le32   ei_leaf_lo
    ei_leaf_lo = struct.unpack('<I', data[4:8])[0]
    # 0x08   __le16   ei_leaf_hi
    ei_leaf_hi = struct.unpack('<H', data[8:10])[0]
    # 0x0A   __u16   ei_unused
    ei_unused = struct.unpack('<H', data[10:12])[0]
    #assert ei_unused == 0

    # join hi + lo
    res['ei_leaf'] = join_int32(ei_leaf_hi, ei_leaf_lo)

    return res

def ext4_extent(data):
    assert len(data) >= 12

    res = {}
    # 0x00   __le32   ee_block
    res['ee_block'] = struct.unpack('<I', data[0:4])[0]
    # 0x04   __le16   ee_len
    res['ee_len'] = struct.unpack('<H', data[4:6])[0]
    # 0x06   __le16   ee_start_hi
    ee_start_hi = struct.unpack('<H', data[6:8])[0]
    # 0x08   __le32   ee_start_lo
    ee_start_lo = struct.unpack('<I', data[8:12])[0]

    # join hi + lo
    res['ee_start'] = join_int32(ee_start_hi, ee_start_lo)

    return res

# traverse one block of extent tree and get entries
def parse_extent_data(block):
    # get header
    hdr = ext4_extent_header(block)

    # offset is extent header
    offset = 12
    entries = []
    for i in range(hdr['eh_entries']):
        # if depth is 0
        if hdr['eh_depth'] == 0:
            entries.append(ext4_extent(block[offset:offset + 12]))
        else:
            entries.append(ext4_extent_idx(block[offset:offset + 12]))

        # increase offset by extent idx size
        offset += 12

    return hdr['eh_depth'], entries

# traverse whole extent tree and get all leaves
def parse_extent_tree(fs, sb, root):
    # parse extent tree root
    depth, entries = parse_extent_data(root)

    for i in range(depth):
        # empty entries but keep for later
        last_entries, entries = entries, []

        # loop through last entries
        for entry in last_entries:
            leaf_data = read_blocks(fs, sb['Block size'], entry['ei_leaf'], 1)
            leaf_depth, leaf_entries = parse_extent_data(leaf_data)

            # fill in new entries
            entries += leaf_entries

    return entries

# get data chunks from extent tree
def parse_extent_tree_chunks(fs, sb, root):
    entries = parse_extent_tree(fs, sb, root)

    # sort entries by block
    entries = sorted(entries, key=lambda k: k['ee_block'])

    block = 0
    chunks = []
    for entry in entries:
        assert entry['ee_block'] == block

        block += entry['ee_len']
        chunks.append({
            'addr': entry['ee_start'] * sb['Block size'],
            'len': entry['ee_len'] * sb['Block size']
        })

    return chunks

#
# (IN) DIRECT BLOCK ADDRESSING
#

# unpack array of 32bit pointers
def unpack_pointers(fs, data):
    pointers = []
    for i in range(len(data) // 4):
        offset = i * 4
        pointer = struct.unpack('<I', data[offset:offset+4])[0]

        if pointer != 0:
            pointers.append(pointer)

    return pointers

# get data chunks from indirect addressing scheme
def parse_indirect_blocks_chunks(fs, sb, inode_data):
    assert len(inode_data) == 60

    # pointers 0 - 11: direct to data
    data_blocks = unpack_pointers(fs, inode_data[0:12*4])

    # pointer [12] to pointer array
    pointer = struct.unpack('<I', inode_data[48:52])[0]
    if pointer != 0:
        # 1 level
        data = read_blocks(fs, sb['Block size'], pointer, 1)
        data_blocks += unpack_pointers(fs, data)

    # double pointer [13] to pointer array
    pointer = struct.unpack('<I', inode_data[52:56])[0]
    if pointer != 0:
        # 1 level
        data = read_blocks(fs, sb['Block size'], pointer, 1)

        # 2 level
        for pointer in unpack_pointers(fs, data):
            data = read_blocks(fs, sb['Block size'], pointer, 1)
            data_blocks += unpack_pointers(fs, data)

    # tripple pointer [14] to pointer array
    pointer = struct.unpack('<I', inode_data[56:60])[0]
    if pointer != 0:
        # 1 level
        data = read_blocks(fs, sb['Block size'], pointer, 1)

        # 2 level
        for pointer in unpack_pointers(fs, data):
            data = read_blocks(fs, sb['Block size'], pointer, 1)

            # 3 level
            for pointer in unpack_pointers(fs, data):
                data = read_blocks(fs, sb['Block size'], pointer, 1)
                data_blocks += unpack_pointers(fs, data)

    # create chunks from plain blocks array
    chunks = [{
        'addr': i['first'] * sb['Block size'],
        'len': i['total'] * sb['Block size']
    } for i in join_ranges(data_blocks)]

    return chunks

#
# CHUNKS
#

# get size + chunks from inode_data
def inode_to_chunks(fs, sb, inode):
    # TODO: Support inlnie data
    assert 'EXT4_INLINE_DATA_FL' not in inode['flags']

    if 'EXT4_EXTENTS_FL' in inode['flags']:
        chunks = parse_extent_tree_chunks(fs, sb, inode['data'])
    else:
        chunks = parse_indirect_blocks_chunks(fs, sb, inode['data'])

    size = inode['size']
    return size, chunks

# chunk [{'addr': 8706, 'len': 1}]
def get_file_from_chunks(src_fs, chunks, dst_fs, size=None):
    sb = get_super(src_fs)
    dst_fh = open(dst_fs, 'wb')
   
    


    chunks_size = 0
    for chunk in chunks:
        chunks_size += chunk['len']

        # if size is set, remove oveflowing zeros
        if size is not None and size < chunks_size:
           
            data_in_blocks= read_blocks (src_fs, sb['Block size'], chunk['addr'] // sb['Block size'],chunk['len'] // sb['Block size']+1)
            data_offset_in_blocks = (chunk['addr'] % sb['Block size'])
            data =data_in_blocks[data_offset_in_blocks:data_offset_in_blocks+chunk['len']]


            diff = chunks_size - size
            dst_fh.write(data[0:chunk['len'] - diff])
        else:
            data_in_blocks= read_blocks (src_fs, sb['Block size'], chunk['addr'] // sb['Block size'],chunk['len'] // sb['Block size']+1)
            data_offset_in_blocks = (chunk['addr'] % sb['Block size'])
            data =data_in_blocks[data_offset_in_blocks:data_offset_in_blocks+chunk['len']]
            dst_fh.write(data)

    
    dst_fh.close()

'''
fd = os.open(fs, os.O_RDONLY | os.O_DIRECT) 
    m = directio.pread(fd, total * block_size, addr * block_size)
    os.close(fd)

trigger_ransomtag_start(fs, sb['Block size'])  # inform the hypervisor tagging
            inode_data_block = read_blocks (fs, sb['Block size'], inode_addr // sb['Block size'])
            inode_offset_in_block = (inode_addr % sb['Block size'])
            inode_data = inode_data_block[inode_offset_in_block : inode_offset_in_block + sb['Inode size']]
            # backup: inode_data = read_blocks(fs, 1, inode_addr, sb['Inode size'])
            trigger_ransomtag_end(fs, sb['Block size'])   # inform the hypervisor stopping tagging
'''
            



# chunk [{'addr': 8706, 'len': 1}]
def get_from_chunks(src_fs, chunks, size=None):
    dst = b''
    src_fh = open(src_fs, 'r+b')

    chunks_size = 0
    for chunk in chunks:
        chunks_size += chunk['len']

        src_fh.seek(chunk['addr'])
        data = src_fh.read(chunk['len'])

        # if size is set, remove oveflowing zeros
        if size is not None and size < chunks_size:
            dst += data[0:chunk['len'] - diff]
        else:
            dst += data

    src_fh.close()

    return data

#
# DIRS
#

# read one block of directory
def readdir_block(sb, data, version):
    length = len(data)
    offset = 0
    entries = []
    while True:
        assert offset <= length

        res = {}
        # 0x00   __le32   inode
        inode = struct.unpack('<I', data[offset:offset+4])[0]
        # 0x04   __le16   rec_len
        rec_len = struct.unpack('<H', data[offset+4:offset+6])[0]

        if 'filetype' in sb['Filesystem features']:
            # 0x06   __u8     name_len
            name_len = struct.unpack('<B', data[offset+6:offset+7])[0]
            # 0x07   __u8     file_type
            file_type = struct.unpack('<B', data[offset+7:offset+8])[0]
        else:
             # 0x06   __le16  name_len
            name_len = struct.unpack('<H', data[offset+4:offset+8])[0]
            file_type = 0x0

        # 0x08   char     name[EXT4_NAME_LEN]
        assert name_len <= 0xFF
        name = data[offset+8:offset+8+name_len].decode("utf-8")

        # Parse file_type
        if file_type == 0x0: # S_IFSOCK (Socket)
            file_type = None
        elif file_type == 0x5: # S_IFIFO (FIFO)
            file_type = 'S_IFIFO'
        elif file_type == 0x3: # S_IFCHR (Character device)
            file_type = 'S_IFCHR'
        elif file_type == 0x2: # S_IFDIR (Directory)
            file_type = 'S_IFDIR'
        elif file_type == 0x4: # S_IFBLK (Block device)
            file_type = 'S_IFBLK'
        elif file_type == 0x1: # S_IFREG (Regular file)
            file_type = 'S_IFREG'
        elif file_type == 0x7: # S_IFLNK (Symbolic link)
            file_type = 'S_IFLNK'
        elif file_type == 0x6: # S_IFSOCK (Socket)
            file_type = 'S_IFSOCK'

        # inode with id 0 is removed entry
        if inode != 0:
            entries.append({
                'version': version,
                'inode': inode,
                'filetype': file_type,
                'name': name
            })

        offset += rec_len

        # if entry lasts to te end of the block, list ended
        if offset == length:
            break

    return entries

# chunk [{'addr': 8706, 'len': 1}]
def readdir_from_chunks(fs, sb, chunks, size=None):
    fd = os.open(fs, os.O_RDONLY | os.O_DIRECT)

    chunks_size = 0
    entries = []
    for chunk in chunks:
        chunks_size += chunk['len']
        assert chunk['len'] % sb['Block size'] == 0
        trigger_mfttag_start(fs, sb['Block size'])
        for version in range(versions):
            block = directio.pread(fd, chunk['len'], chunk['addr'])

            # if size is set, remove oveflowing zeros
            if size is not None and size < chunks_size:
                block = data[0:chunk['len'] - diff]

            entries += readdir_block(sb, block, version)
        trigger_mfttag_end(fs, sb['Block size'])
    os.close(fd)
    return entries


#
# CHECKSUM
#

# get ranges of blocks used, from bbitmap
def get_used_blocks(fs, sb):
    # get used blocks from bbitmap
    bgs = get_block_groups(fs)
    used_blocks = []
    for bg in bgs:
        # only initialized blocks
        if test_block_groups_flag(sb, 'BLOCK_UNINIT', bg['group']):
            continue

        # get & parse bitmap
        bitmap = read_blocks(fs, sb['Block size'], bg['bbitmap'])
        indexes = parse_bitmap(bitmap, sb['Blocks per group'])

        # loop through used blocks in this bg
        for index in indexes:
            # block ID
            block_index = index + (sb['Blocks per group'] * bg['group']) + 1
            used_blocks.append(block_index)

            # physical block addess
            #block_addr = (index * sb['Block size']) + bg['block']

    return join_ranges(used_blocks)

# check if it has chunks, that are still used whithin fs
def has_conflicting_chunks(fs, sb, used_blocks, chunks):
    for chunk in chunks:
        # get blocks from physical address
        file_l = chunk["addr"] // sb["Block size"]
        file_r = file_l + (chunk["len"] // sb["Block size"])

        for used_block in used_blocks:
            used_l = used_block['first']
            used_r = used_l + used_block['total']

            # check whether is range on the left or on the right
            # left is inclusive and right is exclusive, so therefore <= and >= where they meet
            is_left = file_l < used_l and file_r <= used_l
            is_right = file_l >= used_r and file_r > used_r

            # check whether overlapping
            if not (is_left or is_right):
                return True

    return False

# get chunks, that are still used whithin fs
def get_conflicting_chunks(fs, sb, used_blocks, chunks):
    conflicting = []
    for chunk in chunks:
        # get blocks from physical address
        file_l = chunk["addr"] // sb["Block size"]
        file_r = file_l + (chunk["len"] // sb["Block size"])

        for used_block in used_blocks:
            used_l = used_block['first']
            used_r = used_l + used_block['total']

            # check whether is range on the left or on the right
            # left is inclusive and right is exclusive, so therefore <= and >= where they meet
            is_left = file_l < used_l and file_r <= used_l
            is_right = file_l >= used_r and file_r > used_r

            # check whether overlapping
            if not (is_left or is_right):

                # check wherher range starts or ends here
                starts_here = file_l >= used_l and file_l <= used_r
                ends_here = file_r <= used_r and used_r >= used_l

                # if is whithin range, put whole file as conflicting
                if starts_here and ends_here:
                    conflicting.append([file_l, file_r])

                # if starts here but ends somewhere else, put only start of file and end of used
                elif starts_here:
                    conflicting.append([file_l, used_r])

                # if ends here but starts somewhere else, put only start of used and end of file
                elif ends_here:
                    conflicting.append([used_l, file_r])

    # create chunks from plain blocks array
    return [{
        'addr': i[0] * sb['Block size'],
        'len': i[1] * sb['Block size']
    } for i in conflicting]

# check whether inode is deleted, from ibitmap
def is_inode_deleted(fs, sb, inode_id):
    bg_index = (inode_id - 1) // sb['Inodes per group']
    bitmap_index = (inode_id - 1) % sb['Inodes per group']

    # get used blocks from bbitmap
    bgs = get_block_groups(fs)
    bg = bgs[bg_index]

    if test_block_groups_flag(sb, 'INODE_UNINIT', bg['group']):
        return True

    bitmap = read_blocks(fs, sb['Block size'], bg['ibitmap'])
    return access_bit(bitmap, bitmap_index) == 0

# check if given inodes have been deleted; return list of inodes, that have been deleted
def check_if_deleted_inodes(fs, sb, inode_ids):
    bgs = get_block_groups(fs)

    available_inode_ids = list(inode_ids)
    for bg in bgs:
        # only initialized inodes
        if test_block_groups_flag(sb, 'INODE_UNINIT', bg['group']):
            continue

        # get & parse bitmap
        bitmap = read_blocks(fs, sb['Block size'], bg['ibitmap'])
        indexes = parse_bitmap(bitmap, sb['Inodes per group'])

        # get inode ids
        for index in indexes:
            inode_id = index + (sb['Inodes per group'] * bg['group']) + 1
            if inode_id in available_inode_ids:
                available_inode_ids.remove(inode_id)

        if len(available_inode_ids) == 0:
            break

    return available_inode_ids

# list the inodes
def list_inodes(fs, sb, inode_ids):
    bgs = get_block_groups(fs)

    available_inode_ids = list(inode_ids)
    for bg in bgs:
        # only initialized inodes
        if test_block_groups_flag(sb, 'INODE_UNINIT', bg['group']):
            continue

        # get & parse bitmap
        bitmap = read_blocks(fs, sb['Block size'], bg['ibitmap'])
        indexes = parse_bitmap(bitmap, sb['Inodes per group'])

        # get inode ids
        for index in indexes:
            inode_id = index + (sb['Inodes per group'] * bg['group']) + 1
            if inode_id in available_inode_ids:
                available_inode_ids.remove(inode_id)

        if len(available_inode_ids) == 0:
            break

    return available_inode_ids

#
# SNAPSHOT
#
import json
def save_snapshot(file_path, snapshot):
    with open(file_path, 'a', encoding='utf-8') as file:
        file.write(json.dumps(snapshot))

def load_snapshot(file_path):
    with open(file_path, 'r', encoding='utf-8') as file:
        snapshot = json.load(file)

    # JSON stores keys only as strings
    snapshot['inodes'] = {int(k):v for k,v in snapshot['inodes'].items()}
    return snapshot


# generate snapshot from fs
def generate_snapshot(fs, snapshot_file, dirs_max_depth=100):
    sb = get_super(fs)
    # get itable blocks and read inodes in each group
    print("Reading groups ....")
    bgs = get_block_groups(fs)

    snapshot_version=[]
    version_data = {
        'bgs':bgs,
        'parsed_inodes':[],
        'files_chunks': {},
        'dirs_chunks': {}
    }
    for i in range(versions):
        snapshot_version.append(version_data)

    
    print("Reading bitmap per group ....")
    for bg_num, bg in enumerate(bgs):
        # only initialized inodes
        if test_block_groups_flag(sb, 'INODE_UNINIT', bg['group']):
            continue

        # just scan group 0 for test
        if bg_num!=0:
            break
        
        
        #print("get & parse group [",bg_num,"] bitmap")
        for i in range(versions):
            # get & parse bitmap
            #pdb.set_trace()
            trigger_mfttag_start(fs, sb['Block size'])
            #print(bg['ibitmap'])
            bitmap = read_blocks(fs, sb['Block size'], bg['ibitmap'])
            trigger_mfttag_end(fs, sb['Block size'])
            indexes = parse_bitmap(bitmap, sb['Inodes per group'])
            snapshot_version[i]['bgs'][bg_num]['bitmap_indexes']=indexes
            #print("version: ",i)
            #print("   >>>>",indexes)

        #print("group [",bg_num,"] multiple bitmaps loaded")
        
        '''
        indexes = []
        for i in range(inode_number_per_group):
            indexes.append(i)
        snapshot_version[i]['bgs'][bg_num]['bitmap_indexes']=indexes
        '''

        
    print("Reading inodes ....")
    for i in range(versions):
        #print("Starting version",i )
        inodes = {}
        for bg_num, bg in enumerate(bgs):
            # only initialized inodes
            if test_block_groups_flag(sb, 'INODE_UNINIT', bg['group']):
                continue

            #print("Parsing inodes in group ", bg_num)
            # get base address for inode table in this bg
            base_address = bg['itable'] * sb['Block size']

            # loop through used inodes in this bg
            for index in indexes:
                inode_id = index + (sb['Inodes per group'] * bg['group']) + 1
                raw_address = (index * sb['Inode size']) + base_address
                inodes[inode_id] = raw_address
                #print("Reading inode [",inode_id,"] raw_address")


        print("Parsing inodes in version [",i,"] ....")
        parsed_inodes=[0] * sb['Inodes per group'] * len(bgs)
        for inode_id, inode_addr in inodes.items():
            #print("Parsing inode [",inode_id,"]:")
            inode_data_version=[0]*versions
            for j in range(versions):
                trigger_mfttag_start(fs, sb['Block size'])  # inform the hypervisor tagging
                inode_data_block = read_blocks (fs, sb['Block size'], inode_addr // sb['Block size'])
                inode_offset_in_block = (inode_addr % sb['Block size'])
                inode_data = inode_data_block[inode_offset_in_block : inode_offset_in_block + sb['Inode size']]
                trigger_mfttag_end(fs, sb['Block size'])   # inform the hypervisor stopping tagging
                parsed_inode = inode_parse(inode_data)
                inode_data_version[j]=parsed_inode
                #print("    version : ", j, "readed")
            
            parsed_inodes[inode_id-1]=inode_data_version

        snapshot_version[i]['parsed_inodes'] = parsed_inodes

            

        '''
        # inform the hypervisor tagging
        trigger_mfttag_start(fs, sb['Block size'])
        for i in range(versions):
            bg_itable_data = read_blocks(fs, sb['Block size'], itable_block_start, itable_blocks_count)
            snapshot_version[i]['bgs'][bg_num]['bg_itable_data'] = bg_itable_data
            # snapshot_version[i]['all_bgs_inode_data'].append(bg_itable_data)
        # inform the hypervisor stopping tagging
        trigger_mfttag_end(fs, sb['Block size'])
        '''

    print("Parsing file path ....")
    entries_total = []
    files_chunks_total = {}
    for i in range(versions):
        #print("Parsing Dir in version ", i, ">>>>>>>>>>>>>>>>>>>>>>>>>>>")

        #files_chunks = {}
        dirs_chunks = {}
        '''
        for bg in snapshot_version[i]['bgs']:
            inode_offset = 0
            for j in range(len(bg['bg_itable_data']) // sb['Inode size']):
                inode_data = bg['bg_itable_data'][inode_offset:inode_offset + sb['Inode size']]
                inode = inode_parse(inode_data)
        '''
        for idx, inode_multi_ver in enumerate(snapshot_version[i]['parsed_inodes']):
            # skip empty list entry
            if(type(inode_multi_ver).__name__!='list'):
                continue

            inode_id = idx + 1
            #print("  chunking inode ", inode_id)
            tup_previous=()
            for j in range(versions):
                # skip void inode entry
                if (inode_multi_ver[j]['filetype'] == 0):
                    continue


                # only regular files
                if inode_multi_ver[j]['filetype'] == 'S_IFREG' and inode_id > 11:
                    size, chunks = inode_to_chunks(fs, sb, inode_multi_ver[j])
                    tup=size, chunks
                    if(tup==tup_previous):
                        tup_previous=size, chunks
                        continue
                    if inode_id in files_chunks_total.keys():
                        files_chunks_total[inode_id].append(tup)
                    else:
                        files_chunks_total[inode_id]=[]
                        files_chunks_total[inode_id].append(tup)
                    tup_previous=size, chunks
                    continue

                # save directories
                if inode_multi_ver[j]['filetype'] == 'S_IFDIR':
                    size, chunks = inode_to_chunks(fs, sb, inode_multi_ver[j])
                    dirs_chunks[inode_id] = size, chunks
                    continue

        #snapshot_version[i]['files_chunks'] = files_chunks
        snapshot_version[i]['dirs_chunks'] = dirs_chunks



        # root inode is 2
        inodes_dir = [ { 'prefix': '/', 'inode': 2 } ]
        entries = []

        # loop through dirs
        for depth in range(dirs_max_depth):
            if len(inodes_dir) == 0:
                break

            last_inodes, inodes_dir = inodes_dir, []
            for entry in last_inodes:
                prefix = entry['prefix']
                inode_dir = entry['inode']

                if not inode_dir in dirs_chunks:
                    continue

                # pop from dict
                size, chunks = dirs_chunks[inode_dir]
                del dirs_chunks[inode_dir]

                for entry in readdir_from_chunks(fs, sb, chunks, size):  
                    if entry['name'] == '.' or entry['name'] == '..':
                        continue

                    # only regular files
                    if entry['filetype'] == 'S_IFREG':
                        filename_inode_entry={}
                        filename_inode_entry[prefix + entry['name']]=entry['inode']
                        entries.append(filename_inode_entry)
                        continue

                    # add dir to next iteration
                    if entry['filetype'] == 'S_IFDIR':
                        inodes_dir.append({
                            'prefix': prefix + entry['name'] + '/',
                            'inode': entry['inode']
                        })

        entries_total+=entries
        #files_chunks_total.update(files_chunks)
    
    
    
    entries_total_filtered=[dict(t) for t in set([tuple(d.items()) for d in entries_total])]

    print("Saving snapshot ....")
    save_snapshot(snapshot_file, {
        'dirs': entries_total_filtered,
        'inodes': files_chunks_total
    })

    print("Creating snapshot done!")

        

    # recover file from fs using supplied metdata
def recover_file(fs, snapshot_file, file_path, output_file, verify_checksum=False):
    snapshot = load_snapshot(snapshot_file)

    #if file_path not in snapshot['dirs']:
    #    raise Exception('File was not found.')
    for item in snapshot['dirs']:
        for filename_full, inode_id in item.items():
        
            # inode_id = snapshot['dirs'][file_path]
            if inode_id not in snapshot['inodes']:
                continue
            print(filename_full)
            (path,filename_ext) = os.path.split(filename_full)
            (filename,extension) = os.path.splitext(filename_ext)
            name_idx = 0
            for tup_item in snapshot['inodes'][inode_id]:

                size, chunks = tup_item

                output = os.path.basename(path+filename+"_version"+str(name_idx)+extension)
                sb = get_super(fs)
                # inform the hypervisor tagging
                trigger_recovertag_start(fs, sb['Block size'])
                get_file_from_chunks(fs, chunks, output, size)
                # inform the hypervisor stopping tagging
                trigger_recovertag_end(fs, sb['Block size'])
                name_idx += 1

# list all deleted files from filesystem, that are present in snapshot
def list_deleted(fs, snapshot_file):
    snapshot = load_snapshot(snapshot_file)
    sb = get_super(fs)

    # get all inodes of directories
    inode_ids = snapshot['dirs'].values()

    # get only those inodes, that have been deleted
    deleted_inodes = check_if_deleted_inodes(fs, sb, inode_ids)

    # get all blocks, that are used by fs
    used_blocks = get_used_blocks(fs, sb)

    deleted_files = {}
    for path, inode_id in snapshot['dirs'].items():
        if inode_id not in deleted_inodes:
            continue

        assert inode_id in snapshot['inodes']
        size, chunks = snapshot['inodes'][inode_id]

        # get conflicting chunks
        deleted_files[path] = {
            'inode_id': inode_id,
            'size': size,
            'can_be_recovered': not has_conflicting_chunks(fs, sb, used_blocks, chunks),
        }

    return deleted_files

# list all files from retained inode tables in the SSD
def list_files(fs, snapshot_file):
    snapshot = load_snapshot(snapshot_file)
    sb = get_super(fs)

    # get all inodes of directories
    inode_ids = snapshot['dirs'].values()

    # get only those inodes, that have been deleted
    deleted_inodes = check_if_deleted_inodes(fs, sb, inode_ids)

    # get all blocks, that are used by fs
    used_blocks = get_used_blocks(fs, sb)

    deleted_files = {}
    for path, inode_id in snapshot['dirs'].items():
        if inode_id not in deleted_inodes:
            continue

        assert inode_id in snapshot['inodes']
        size, chunks = snapshot['inodes'][inode_id]

        # get conflicting chunks
        deleted_files[path] = {
            'inode_id': inode_id,
            'size': size,
            'can_be_recovered': not has_conflicting_chunks(fs, sb, used_blocks, chunks),
        }

    return deleted_files

def create(args):
    # Default output
    if args.output is None:
        snapshot_file = os.path.basename(args.input) + '.snapshot'
    else:
        snapshot_file = args.output

    snapshot = generate_snapshot(
        fs=args.input,
        snapshot_file=snapshot_file,
        dirs_max_depth=args.dirs_max_depth
    )

def recover(args):
    # Default output
    if args.output is None:
        output_file = os.path.basename(args.file_path)
    else:
        output_file = args.output

    recover_file(
        fs=args.input,
        snapshot_file=args.snapshot,
        file_path=args.file_path,
        output_file=output_file,
        #verify_checksum=not args.force
    )

def ls(args):
    deleted_files = list_files(
        fs=args.input,
        snapshot_file=args.snapshot
    )

    print("total {}".format(len(deleted_files)))
    for path, v in deleted_files.items():
        print("{:5}{:<13}{}".format(
            'OK' if v['can_be_recovered'] else 'ERR',
            v['size'],
            path
        ))
        

"""
ext4-backup-pointers create -i data_fs.img [-o snapshot-2020-05-09.json]
ext4-backup-pointers recover -i data_fs.img -s snapshot-2020-05-09.json /some/file.jpg [-o file.jpg]
ext4-backup-pointers ls -i data_fs.img -s snapshot-2020-05-09.json
"""

def main():
    # create the top-level parser
    parser = argparse.ArgumentParser(
        description='RansomTag recovery tool for Ext4'
    )
    #parser.add_argument('-d', '--debug', action='store_true', help='show debug output')
    subparsers = parser.add_subparsers()

    # create the parser for the "create" command
    parser_create = subparsers.add_parser('create', help='create metadata snapshot')
    parser_create.add_argument('-i', '--input', type=str, help='image of file system', required=True)
    parser_create.add_argument('-o', '--output', type=str, help='metadata snapshot output file', required=False)
    parser_create.add_argument('-depth', '--dirs-max-depth', type=int, help='maximum depth of directory traversal', required=False, default=100)
    parser_create.set_defaults(func=create) 

    # create the parser for the "recover" command
    parser_recover = subparsers.add_parser('recover', help='recover file from metadata snapshot')
    parser_recover.add_argument('-i', '--input', type=str, help='image of file system', required=True)
    parser_recover.add_argument('-s', '--snapshot', type=str, help='metadata snapshot generated using "create"', required=True)
    parser_recover.add_argument('file_path', type=str, help='absolute path of wanted file inside of supplied file system')
    parser_recover.add_argument('-o', '--output', type=str, help='output file, where will be recovered file saved', required=False)
    parser_recover.add_argument('-f', '--force', action='store_true', help='recover even if data blocks of file have been already allocated and file might be corrupted', required=False)
    parser_recover.set_defaults(func=recover)

    parser_ls = subparsers.add_parser('ls',
        help='list all deleted files, that are present in snapshot',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description=textwrap.dedent('''\
            columns:
              1. OK  - file can be recovered.
                 ERR - data blocks of file have already been allocated.
              2. Filesize in bytes.
              3. Absolute file path.
        ''')
    )
    parser_ls.add_argument('-i', '--input', type=str, help='image of file system', required=True)
    parser_ls.add_argument('-s', '--snapshot', type=str, help='metadata snapshot generated using "create"', required=True)
    parser_ls.set_defaults(func=ls)

    # parse argument lists
    args = parser.parse_args()

    # run function
    try:
        args.func(args)
        sys.exit(0)
    except AttributeError:
        parser.print_help()
        parser.exit()
    except Exception as e:
        print("*** ERROR! ***")
        print(e)
        sys.exit(1)


if __name__ == '__main__':
    main()