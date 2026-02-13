#include "ssu_ext2.h"


/**
 * 경로 분석하여 inode 번호 찾기
 * 
 * @param fd 파일 디스크립터
 * @param sb 슈퍼블록 포인터
 * @param gd 그룹 디스크립터 배열 포인터
 * @param path 검색할 경로
 * @return 경로에 해당하는 inode 번호, 없으면 0 반환
 */
unsigned int	path_to_inode(int fd, struct my_ext2_super_block *sb, 
						  struct my_ext2_group_desc *gd, 
						  const char *path)
{
	if (path == NULL || path[0] == '\0') {
		return 0;
	}
		
	// 루트 디렉토리부터 시작
	unsigned int current_inode = EXT2_ROOT_INO;  // 2
		
	// 절대 경로인지 확인
	if (path[0] == '/') {
		path++;  // '/' 건너뛰기
	}
		
	// 경로가 비어있으면 루트 반환
	if (path[0] == '\0') {
		return current_inode;
	}
		
	// 경로 복사 (strtok이 원본을 수정하므로)
	char path_copy[MAX_PATH];
	strncpy(path_copy, path, MAX_PATH - 1);
	path_copy[MAX_PATH - 1] = '\0';
		
	// 경로 구성 요소별로 처리
	char *token = strtok(path_copy, "/");
	while (token) {
		struct my_ext2_inode inode;
		
		// 현재 inode 정보 읽기
		if (read_inode(fd, current_inode, sb, gd, &inode) < 0) {
			return 0;
		}
		
		// 현재 inode가 디렉토리인지 확인
		if (!S_ISDIR(inode.i_mode)) {
			return 0;
		}
		
		// 현재 디렉토리에서 다음 경로 요소 찾기
		current_inode = find_entry_in_dir(fd, sb, &inode, token);
		if (current_inode == 0) {
			return 0;  // 찾지 못함
		}
		
		token = strtok(NULL, "/");
	}
		
	return current_inode;
}
