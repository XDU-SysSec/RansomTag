# NTFS Usage

1. Use pyinstaller to convert the ransomrecover.py to ransomrecover.exe
2. Run the program in a command terminal with '--save-mft mft' to scan the retained MFT
3. Run the program again with '/dev/diskX --mft mft --pattern "*.jpg" --outdir recovered' to recover the files

# ext4 Usage
1. install directio module
2. run ransomtag-recovery.py with "-i create" parameter to create inode records snapshot
3. run ransomtag-recovery.py with "-i snapshot -recover" parameter to recover files