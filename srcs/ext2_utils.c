#include "ssu_ext2.h"

/**
 * EXT2 파일 시스템의 슈퍼블록을 읽는 함수
 * 
 * @param fd EXT2 이미지 파일 디스크립터
 * @param sb 슈퍼블록 구조체 포인터 (결과 저장)
 * @return 성공 시 0, 실패 시 음수 값 반환
 */
int	read_super_block(int fd, struct my_ext2_super_block *sb)
{
	// 슈퍼블록 위치(1024 바이트)로 이동
	if (lseek(fd, 1024, SEEK_SET) != 1024) {
		#ifdef DEBUG_FUNC
			fprintf(stderr, "lseek failed");
		#endif
		return -1;
	}
		
	// 슈퍼블록 읽기
	ssize_t bytes_read = read(fd, sb, sizeof(struct my_ext2_super_block));
	if (bytes_read != sizeof(struct my_ext2_super_block)) {
		#ifdef DEBUG_FUNC
			fprintf(stderr, "read failed");
		#endif
		return -2;
	}
		
	// 매직 넘버 확인 (유효한 EXT2 파일 시스템인지)
	if (sb->s_magic != EXT2_SUPER_MAGIC) {
		fprintf(stderr, "Wrong ext2 filesystem (magic=0x%04x)\n", sb->s_magic);
		return -3;
	}
		
	return 0;  // 성공
}

/**
 * 슈퍼블록으로부터 블록 크기를 계산하는 함수
 * 
 * @param sb 슈퍼블록 포인터
 * @return 계산된 블록 크기
 */
unsigned int	get_block_size(struct my_ext2_super_block *sb)
{
	return 1024 << sb->s_log_block_size;
}

/**
 * 데이터 블록을 읽는 함수
 * 
 * @param fd 파일 디스크립터
 * @param sb 슈퍼블록 포인터
 * @param block_num 읽을 블록 번호
 * @param buffer 읽은 데이터를 저장할 버퍼
 * @return 성공 시 0, 실패 시 음수 값
 */
int	read_data_block(int fd, struct my_ext2_super_block *sb, 
					unsigned int block_num, unsigned char *buffer)
{
	unsigned int block_size = get_block_size(sb);
	off_t offset = (off_t)block_num * block_size;
		
	if (lseek(fd, offset, SEEK_SET) != offset) {
		#ifdef DEBUG_FUNC
			fprintf(stderr, "lseek failed in read_data_block");
		#endif
		return -1;
	}
		
	ssize_t bytes_read = read(fd, buffer, block_size);
	if (bytes_read != block_size) {
		#ifdef DEBUG_FUNC
			fprintf(stderr, "read failed in read_data_block");
		#endif
		return -2;
	}
		
	return 0;
}

/**
 * inode 정보를 읽는 함수
 * 
 * @param fd EXT2 이미지 파일 디스크립터
 * @param inode_num 읽을 inode 번호
 * @param sb 슈퍼블록 포인터
 * @param gd 그룹 디스크립터 배열 포인터
 * @param inode inode 정보를 저장할 구조체 포인터
 * @return 성공 시 0, 실패 시 음수 값 반환
 */
int	read_inode(int fd, int inode_num, 
			  struct my_ext2_super_block *sb, 
			  struct my_ext2_group_desc *gd, 
			  struct my_ext2_inode *inode)
{
	// inode 번호 유효성 검사
	if (inode_num < 1 || inode_num > sb->s_inodes_count) {
		return -1;
	}
		
	// 블록 크기 계산
	int block_size = get_block_size(sb);
		
	// 블록 그룹 번호 계산
	int inodes_per_group = sb->s_inodes_per_group;
	int group_num = (inode_num - 1) / inodes_per_group;
		
	// 블록 그룹 내 inode 인덱스 계산
	int inode_index = (inode_num - 1) % inodes_per_group;
		
	// 해당 블록 그룹의 디스크립터 가져오기
	struct my_ext2_group_desc *group_desc = &gd[group_num];
		
	// inode 테이블의 시작 블록 번호
	__u32 inode_table = group_desc->bg_inode_table;
		
	// inode 크기 (일반적으로 128바이트)
	__u32 inode_size = 128;
	if (sb->s_rev_level > 0 && sb->s_inode_size > 0) {
		inode_size = sb->s_inode_size;
	}
		
	// inode 오프셋 계산
	off_t offset = (off_t)block_size * inode_table + (off_t)inode_index * inode_size;
		
	// inode 위치로 이동
	if (lseek(fd, offset, SEEK_SET) != offset) {
		#ifdef DEBUG_FUNC
			fprintf(stderr, "lseek failed in read_inode");
		#endif
		return -2;
	}
		
	// inode 정보 읽기
	ssize_t bytes_read = read(fd, inode, sizeof(struct my_ext2_inode));
	if (bytes_read != sizeof(struct my_ext2_inode)) {
		#ifdef DEBUG_FUNC
			fprintf(stderr, "read failed in read_inode");
		#endif
		return -3;
	}
		
	return 0;  // 성공
}

/**
 * 디렉토리 내에서 특정 이름의 엔트리 찾기
 * 
 * @param fd 파일 디스크립터
 * @param sb 슈퍼블록 포인터
 * @param dir_inode 디렉토리 inode 포인터
 * @param name 찾을 엔트리 이름
 * @return 찾은 엔트리의 inode 번호, 못 찾으면 0 반환
 */
unsigned int	find_entry_in_dir(int fd, struct my_ext2_super_block *sb, 
							 struct my_ext2_inode *dir_inode, 
							 const char *name)
{
	unsigned int block_size = get_block_size(sb);
	unsigned char *block = malloc(block_size);
	if (block == NULL) {
		return 0;
	}
		
	// 직접 블록만 처리 (간소화)
	for (int i = 0; i < EXT2_NDIR_BLOCKS; i++) {
		if (dir_inode->i_block[i] == 0) {
			continue;
		}
		
		if (read_data_block(fd, sb, dir_inode->i_block[i], block) < 0) {
			continue;
		}
		
		unsigned int offset = 0;
		while (offset < block_size) {
			struct my_ext2_dir_entry_2 *entry = 
				(struct my_ext2_dir_entry_2 *)(block + offset);
			
			if (entry->inode == 0 || entry->rec_len == 0) {
				break;
			}
			
			// 이름 비교
			if (entry->name_len == strlen(name) && 
				strncmp(entry->name, name, entry->name_len) == 0) {
				unsigned int result = entry->inode;
				free(block);
				return result;
			}
			
			// 중요: 실제 필요한 크기 계산
			unsigned int real_size = 8 + entry->name_len;  // 8바이트 헤더 + 이름 길이
			real_size = (real_size + 3) & ~3;  // 4바이트 정렬
			
			// lost+found의 경우 rec_len이 매우 클 수 있음
			// 다음 위치에 유효한 엔트리가 있는지 확인
			if (entry->rec_len > real_size + 8 && offset + real_size < block_size) {
				struct my_ext2_dir_entry_2 *next = 
					(struct my_ext2_dir_entry_2 *)(block + offset + real_size);
				
				if (next->inode > 0 && next->inode < sb->s_inodes_count) {
					// 다음 엔트리가 유효하면 실제 크기만 사용
					offset += real_size;
				} else {
					// 그렇지 않으면 rec_len 사용
					offset += entry->rec_len;
				}
			} else {
				offset += entry->rec_len;
			}
		}
	}
		
	free(block);
	return 0;  // 찾지 못함
}
