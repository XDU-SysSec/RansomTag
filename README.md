# RansomTag
A Tag-Based Approach Against Crypto Ransomware with Fine-Grained Data Recovery

# Introduction
Ransomware has evolved from an economic nuisance to a national security threat nowadays, which poses a significant risk to users. 
To address this problem, we propose RansomTag, a tag-based approach against crypto ransomware with fine-grained data recovery.  

## Features of RansomTag
- It decouples the ransomware detection functionality from the firmware of the SSD and integrates it into a lightweight hypervisor of Type I. 
Thus, it can leverage the powerful computing capability of the host system and the rich context information, which is introspected from the operating system, to achieve accurate detection of ransomware attacks and defense against potential targeted attacks on SSD characteristics. 
Further, RansomTag is readily deployed onto desktop personal computers due to its parapass-through architecture.

- RansomTag bridges the semantic gap between the hypervisor and the SSD through the tag-based approach proposed by us.
  
- RansomTag is able to keep 100% of the user data overwritten or deleted by ransomware, and restore any single or multiple user files to any versions based on timestamps.

## Implementation
The prototype of RansomTag consists of C code and Python code.

### Detection modules
We modify and integrate an open-source Type I hypervisor to implement the hypervisor-side functionalities of RansomTag. 

### Data backup and recovery modules
We integrate the SSD-side functionalities into an open-source FTL project and port the modified FTL to a development board as the evaluation SSD.

### Recovery tool 
We also develop a Python program tool to recover the attacked files in fine-grained. 

# Usage
The details of usage can be found in individual `README.md` file in each directory.

If you have any questions, please feel free to contact me via email: byma_1@stu.xidian.edu.cn
