#include "fsrec.h"

static struct super_block *sb;   
unsigned int used_inodes = 0;
unsigned int used_znodes = 0;

int fs_zoneinfo(dev_t dev, ino_t ino){

	printf("Printf Zone Information\n");	
	sb = get_super(fs_dev);

	struct inode *pinode;
	if((pinode = get_inode(dev,ino)) == NULL){
		printf("Failed to get Inode Information");
		return 0;
	}	
	
	int index = 0;
	/* there will be 0-6 direct zones in inode */
	printf("Single inDirect Zones \n\n");
	int j = 0;
	for(j = 0; j < pinode->i_ndzones; j++){
			if(pinode->i_zone[j] == 0)
				break;
			printf("%d ",pinode->i_zone[j]);
	}
	
	if(pinode->i_zone[j] == 0)
		return 0;
	
	do_singleindirect(pinode->i_zone[j]);	
	j++;
	
	if(pinode->i_zone[j] == 0)
		return 0;
	
	do_doubleindirect(pinode->i_zone[j]);	
	
	return 0;
}

void do_doubleindirect(zone_t zno){
	
	struct buf *buf;
	buf = get_block(fs_dev,zno,0);
	zone_t *dzone = b_v2_ind(buf);
	printf("Double Indirect Zone \n\n");
	for(int j = 0;j < sb->s_block_size/2; j++){
		do_singleindirect(dzone[j]);
	}

}

void do_singleindirect(zone_t zno){
	struct buf *buf;
	buf = get_block(fs_dev,zno,NORMAL);
	zone_t *szone;
	szone = b_v2_ind(buf);
	printf("Indirect Zone \n\n");
	for(int j = 0; j < sb->s_block_size/2; j++){
		if(szone[j] == 0)
			break;
		printf("%d ",szone[j]);
	}
}

int fs_znodewalker(){
		
	printf("Getting super node from device %llu ...\n\n", fs_dev);
	int type = ZMAP;
		
	sb = get_super(fs_dev);
	read_super(sb);
	
	print_superblock();
	
	bitchunk_t *zmap_disk = (bitchunk_t *) malloc((unsigned) sb->s_zmap_blocks * sb->s_block_size);
	memset(zmap_disk, 0, sb->s_zmap_blocks * sb->s_block_size);
	*zmap_disk |= 1;
	
	get_bitmap(zmap_disk, ZMAP);
	
	get_list_used(zmap_disk, ZMAP);
	
	free(zmap_disk);

	return 0;
}

int fs_inodewalker(){
	
	
	printf("Getting super node from device %llu ...\n\n", fs_dev);
	int type = IMAP;
	
	/* get Super Block in memory */
	sb = get_super(fs_dev);
	
	read_super(sb);
	print_superblock();
	
	bitchunk_t *imap_disk = (bitchunk_t *) malloc((unsigned) sb->s_imap_blocks * sb->s_block_size);
	memset(imap_disk, 0, sb->s_imap_blocks * sb->s_block_size);
	*imap_disk |= 1;
	
	get_bitmap(imap_disk, IMAP);
	
	get_list_used(imap_disk, IMAP);
	
	free(imap_disk);

	return 0;
}

void get_list_used(bitchunk_t *bitmap, int type){ 
/* Get a list of unused blocks/inodes from the zone/inode bitmap */
	int *list;
	int nblk, total;
	bitchunk_t *buf;
	char* chunk;
	unsigned int used = 0;
	if (type == IMAP){
		nblk = sb->s_imap_blocks;
		total  = sb->s_ninodes;
		list = malloc(sizeof(int) * sb->s_ninodes);
		printf("Used Inodes \n\n");
	}
	else{
		nblk = sb->s_zmap_blocks;
		total  = sb->s_zones;
		list = (int *) malloc( sizeof(int) * sb->s_zones );
		printf("Used Blocks \n\n");
	}
	printf("\n\n");
	for (int j = 0; j < FS_BITMAP_CHUNKS(sb->s_block_size) * nblk; j++){
		chunk = int2binstr(bitmap[j]);
		for (int k = 0; k < strlen(chunk); k++){
			if (chunk[k] == '1'){
				list[used] = j * FS_BITCHUNK_BITS + k;
				printf("%d, ", list[used]);
				if (used % 5 == 0){
					printf("\n\n");
				}
				++used;
			}
		}
	}
	if (type == IMAP)
		used_inodes  = used;
	else
		used_znodes = used;
	
	printf("Used %d of %d \n\n", used, total);
}

void get_bitmap(bitchunk_t *bitmap, int type){ /* Get a bitmap (imap or zmap) from disk */
	block_t *list;
	block_t bno;
	int nblk;
	if (type == IMAP){
		bno  = 2;
		nblk = sb->s_imap_blocks;
	}
	else /* type == ZMAP */ {
		bno  = 2 + sb->s_imap_blocks;
		nblk = sb->s_zmap_blocks;
	}
	int i;
	bitchunk_t *p;
	struct buf *bp;
	bitchunk_t *bf;
	p = bitmap;
	for (i = 0; i < nblk; i++, bno++, p += FS_BITMAP_CHUNKS(sb->s_block_size)){
		bp = get_block(fs_dev, bno, 0);
		for (int j = 0; j < FS_BITMAP_CHUNKS(sb->s_block_size); ++j){
			p[j]  = b_bitmap(bp)[j];
		}
	}
}

void print_superblock(){
	printf("ninodes       = %u\n", sb->s_ninodes);
	printf("nzones        = %d\n", sb->s_zones);
	printf("imap_blocks   = %u\n", sb->s_imap_blocks);
	printf("zmap_blocks   = %u\n", sb->s_zmap_blocks);
	printf("log_zone_size = %u\n", sb->s_log_zone_size);
	printf("maxsize       = %d\n", sb->s_max_size);
	printf("block size    = %d\n", sb->s_block_size);
	printf("\n\n");
}

char * int2binstr(unsigned int i){
	
	size_t bits = sizeof(unsigned int) * CHAR_BIT;
	char * str = malloc(bits + 1);
	if(!str) return NULL;
	str[bits] = 0;

	unsigned u = *(unsigned *)&i;
	for(; bits--; u >>= 1)
		str[bits] = u & 1 ? '1' : '0';

	return str;
}
