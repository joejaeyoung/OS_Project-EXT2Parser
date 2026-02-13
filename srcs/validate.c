#include "ssu_ext2.h"
/**
 * tree 명령어 경로 유효성 검사 함수
 * @param path 검사할 경로
 * @return 리턴값 int로 조정해서 0이면 help 출력 -1이면 내부 출력 1이면 성공
 */
int	validate_tree_path(const char *path)
{
	// NULL 경로 또는 빈 경로 검사
	if (path == NULL || path[0] == '\0') {
		#ifdef DEBUG_VALID
			fprintf(stderr, "Error: Path is empty\n");
		#endif
		return 0;
	}
		
	int ext2_fd = open(img_path, O_RDONLY);

		
	// 슈퍼블록 읽기
	struct my_ext2_super_block sb;
	if (read_super_block(ext2_fd, &sb) < 0) {
		#ifdef DEBUG_VALID
			fprintf(stderr, "Error reading superblock\n");
		#endif
		close(ext2_fd);
		return 0;
	}
		
	// 블록 그룹 디스크립터 읽기
	int block_size = get_block_size(&sb);
	int block_groups_count = (sb.s_blocks_count + sb.s_blocks_per_group - 1) 
							/ sb.s_blocks_per_group;
		
	// 블록 그룹 디스크립터 테이블 크기
	int gdt_size = block_groups_count * sizeof(struct my_ext2_group_desc);
	struct my_ext2_group_desc *gd = (struct my_ext2_group_desc *)malloc(gdt_size);
		
	// 블록 그룹 디스크립터 테이블 위치 (슈퍼블록 바로 다음)
	off_t gdt_offset = (sb.s_first_data_block + 1) * block_size;
	if (lseek(ext2_fd, gdt_offset, SEEK_SET) != gdt_offset) {
		#ifdef DEBUG_VALID
			fprintf(stderr, "Error seeking to group descriptor table\n");
		#endif
		free(gd);
		close(ext2_fd);
		return 0;
	}
		
	// 블록 그룹 디스크립터 테이블 읽기
	if (read(ext2_fd, gd, gdt_size) != gdt_size) {
		#ifdef DEBUG_VALID
			fprintf(stderr, "Error reading group descriptor table\n");
		#endif
		free(gd);
		close(ext2_fd);
		return 0;
	}
		
	// 경로의 inode 번호 얻기
	unsigned int inode_num = path_to_inode(ext2_fd, &sb, gd, path);
	if (inode_num == 0) {
		#ifdef DEBUG_VALID
			fprintf(stderr, "Error: Path not found: %s\n", path);
		#endif
		free(gd);
		close(ext2_fd);
		return 0;
	}
		
	// inode 정보 읽기
	struct my_ext2_inode inode;
	if (read_inode(ext2_fd, inode_num, &sb, gd, &inode) < 0) {
		#ifdef DEBUG_VALID
			fprintf(stderr, "Error: Failed to read inode\n");
		#endif
		free(gd);
		close(ext2_fd);
		return 0;
	}
		
	// 파일인지 확인 (디렉토리가 아니어야 함)
	if (!S_ISDIR(inode.i_mode)) {
		fprintf(stdout, "Error: '%s' is not directory\n", path);
		free(gd);
		close(ext2_fd);
		return -1;
	}
		
	// 메모리 해제 및 파일 닫기
	free(gd);
	close(ext2_fd);
		
	return 1;  // 유효한 파일 경로
}
