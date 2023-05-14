### Notes about fs and thoughts about questions in "Understanding Unix-Linux Programming"

#### Q4.4
Q. The text stated the kernel had to locate a free inode and free disk blocks when it created a new file. How does the kernel know which blocks are free? How does the kernel know which inodes are free?

A. How does the kernel know which inodes are free?
In Linux 0.11 source code, the `new_inode` function has the following code snippet:
```C
for (i=0 ; i<8 ; i++)
		if (bh=sb->s_imap[i])
			if ((j=find_first_zero(bh->b_data))<8192)
				break;
	if (!bh || j >= 8192 || j+i*8192 > sb->s_ninodes) {
		iput(inode);
		return NULL;
	}
```