#include "ssu_ext2.h"

/**
*
*트리 노드 생성 함수
*
*@param name 노드 이름
*@param inode_num inode 번호
*@param file_type 파일 타입
*@param size 파일 크기
*@param permissions 파일 권한
*@return 생성된 트리 노드 포인터
*/
DirTreeNode*	create_tree_node(const char* name, int inode_num, int file_type, unsigned int size, unsigned int permissions)
{
	DirTreeNode* node = (DirTreeNode*)malloc(sizeof(DirTreeNode));
		
	strncpy(node->name, name, MAX_FILE_NAME);
	node->name[MAX_FILE_NAME] = '\0';
	node->inode_num = inode_num;
	node->file_type = file_type;
	node->size = size;
	node->permissions = permissions;
	node->first_child = NULL;
	node->next_sibling = NULL;
		
	return node;
}

/**
*
*디렉토리 노드에 자식 노드 추가 함수
*
*@param parent 부모 노드 포인터
*@param child 추가할 자식 노드 포인터
*/
void	add_child_node(DirTreeNode* parent, DirTreeNode* child)
{
	if (parent == NULL || child == NULL) {
		return;
	}
		
	if (parent->first_child == NULL) {
		// 첫 번째 자식이 없으면 바로 추가
		parent->first_child = child;
	} else {
		// 마지막 형제 노드를 찾아서 추가
		DirTreeNode* sibling = parent->first_child;
		while (sibling->next_sibling != NULL) {
			sibling = sibling->next_sibling;
		}
		sibling->next_sibling = child;
	}
}

/**
*
*디렉토리 블록 처리 함수
*
*@param fd 파일 디스크립터
*@param sb 슈퍼블록 포인터
*@param gd 그룹 디스크립터 배열 포인터
*@param block_num 처리할 블록 번호
*@param parent_node 부모 트리 노드 포인터
*@param recursive 재귀 옵션 플래그
*@param dir_count 디렉토리 개수 포인터
*@param file_count 파일 개수 포인터
*@return 발견된 엔트리 수
*/
int	process_directory_block(int fd, struct my_ext2_super_block *sb, 
						   struct my_ext2_group_desc *gd,
						   unsigned int block_num, DirTreeNode *parent_node,
						   int recursive, int *dir_count, int *file_count)
{
	int entries_found = 0;
	unsigned int block_size = get_block_size(sb);
	unsigned char *block_buf = (unsigned char *)malloc(block_size);
		
	#ifdef DEBUG_TREE
		printf("Reading directory block %u\n", block_num);
		debug_directory_block(block_buf, block_size);
	#endif

	// 블록 데이터 읽기
	if (read_data_block(fd, sb, block_num, block_buf) < 0) {
		#ifdef DEBUG_TREE
			printf("Failed to read block %u\n", block_num);
		#endif
		free(block_buf);
		return 0;
	}
		
	// 블록 내의 디렉토리 엔트리 처리
	unsigned int offset = 0;
	while (offset < block_size) {
		struct my_ext2_dir_entry_2 *entry = 
			(struct my_ext2_dir_entry_2 *)(block_buf + offset);
		
		// 엔트리 종료 확인
		if (entry->inode == 0 || offset + 8 > block_size) {
			break;
		}
		
		// 엔트리 이름 추출
		if (entry->name_len == 0 || entry->name_len > 255 || offset + 8 + entry->name_len > block_size) {
			// 유효하지 않은 name_len 값이면 다음 위치로 이동
			offset += 4;  // 최소 간격으로 이동
			continue;
		}
		
		char entry_name[MAX_FILE_NAME + 1];
		memset(entry_name, 0, sizeof(entry_name));
		strncpy(entry_name, entry->name, entry->name_len);
		entry_name[entry->name_len] = '\0';
		
		#ifdef DEBUG_TREE
			printf("Found entry in block %u at offset %u: name='%s', inode=%u, rec_len=%u, name_len=%u, file_type=%u\n", 
			  block_num, offset, entry_name, entry->inode, entry->rec_len, entry->name_len, entry->file_type);
		#endif

		// ".", "..", "lost+found" 제외
		if (strcmp(entry_name, ".") != 0 && 
			strcmp(entry_name, "..") != 0 && 
			strcmp(entry_name, "lost+found") != 0) {
			
			// 엔트리의 inode 정보 읽기
			struct my_ext2_inode entry_inode;
			if (read_inode(fd, entry->inode, sb, gd, &entry_inode) == 0) {
				// 파일 타입 확인
				int is_dir = S_ISDIR(entry_inode.i_mode);
				
				#ifdef DEBUG_TREE
					printf("Entry %s is %s, mode: %o, size: %u\n", 
					  entry_name, is_dir ? "directory" : "file", 
					  entry_inode.i_mode, entry_inode.i_size);
				#endif

				// 트리 노드 생성
				DirTreeNode *node = create_tree_node(
					entry_name, entry->inode, entry_inode.i_mode, 
					entry_inode.i_size, entry_inode.i_mode & 0xFFF
				);
				
				if (node != NULL) {
					// 부모 노드에 추가
					add_child_node(parent_node, node);
					
					// 파일/디렉토리 카운트 증가
					if (is_dir) {
						(*dir_count)++;
						
						// 재귀 옵션이 켜져 있고 디렉토리인 경우 하위 디렉토리 처리
						if (recursive) {
							int sub_result = read_directory_entries(
								fd, sb, gd, entry->inode, node, recursive);
						}
					} else {
						(*file_count)++;
					}
					
					entries_found++;
				}
			}
		}
		
		// 다음 엔트리로 이동
		// 디렉토리 엔트리의 실제 필요 크기 계산
		unsigned int real_size = 8 + entry->name_len;  // 기본 헤더(8바이트) + 이름 길이
		real_size = (real_size + 3) & ~3;  // 4바이트 정렬
		
		// 다음 엔트리가 있는지 확인
		if (entry->rec_len > real_size + 8 && offset + real_size < block_size) {
			// 실제 크기 이후 위치에서 다음 엔트리 확인
			struct my_ext2_dir_entry_2 *next = 
				(struct my_ext2_dir_entry_2 *)(block_buf + offset + real_size);
			
			// 다음 위치에 유효한 엔트리가 있는지 확인
			if (next->inode > 0 && next->inode < sb->s_inodes_count &&
				next->rec_len >= 8 && next->rec_len <= block_size - (offset + real_size) &&
				next->name_len > 0 && next->name_len <= 255) {
				// 다음 엔트리가 유효하면 실제 크기만큼만 이동
				#ifdef DEBUG_TREE
					printf("Found hidden entry after %s at offset %u\n", 
					  entry_name, offset + real_size);
				#endif

				offset += real_size;
			} else {
				// 다음 엔트리가 유효하지 않으면 rec_len 값 사용
				offset += entry->rec_len;
			}
		} else {
			// 다음 엔트리가 없거나 rec_len이 적절한 경우
			offset += entry->rec_len;
		}
	}
		
	free(block_buf);
	return entries_found;
}

/**
*단일 간접 블록 처리 함수
*
*@param fd 파일 디스크립터
*@param sb 슈퍼블록 포인터
*@param gd 그룹 디스크립터 배열 포인터
*@param indirect_block_num 단일 간접 블록 번호
*@param parent_node 부모 트리 노드 포인터
*@param recursive 재귀 옵션 플래그
*@param dir_count 디렉토리 개수 포인터
*@param file_count 파일 개수 포인터
*@return 발견된 엔트리 수
*/
int	process_indirect_block(int fd, struct my_ext2_super_block *sb, 
						  struct my_ext2_group_desc *gd,
						  unsigned int indirect_block_num, DirTreeNode *parent_node,
						  int recursive, int *dir_count, int *file_count)
{
	int entries_found = 0;
	unsigned int block_size = get_block_size(sb);
	unsigned char *block_buf = (unsigned char *)malloc(block_size);
		
	#ifdef DEBUG_TREE
		printf("Processing single indirect block %u\n", indirect_block_num);
	#endif

	// 간접 블록 데이터 읽기
	if (read_data_block(fd, sb, indirect_block_num, block_buf) < 0) {
		free(block_buf);
		return 0;
	}
		
	// 간접 블록에서 블록 번호 배열 읽기
	unsigned int *block_ptrs = (unsigned int *)block_buf;
	unsigned int ptrs_per_block = block_size / sizeof(unsigned int);
		
	// 각 블록 번호에 대해 디렉토리 블록 처리
	for (unsigned int i = 0; i < ptrs_per_block; i++) {
		if (block_ptrs[i] == 0) {
			continue;  // 빈 포인터 건너뛰기
		}
		
		entries_found += process_directory_block(fd, sb, gd, block_ptrs[i], 
											   parent_node, recursive, dir_count, file_count);
	}
		
	free(block_buf);
	return entries_found;
}

/**
*이중 간접 블록 처리 함수
*
*@param fd 파일 디스크립터
*@param sb 슈퍼블록 포인터
*@param gd 그룹 디스크립터 배열 포인터
*@param double_indirect_block_num 이중 간접 블록 번호
*@param parent_node 부모 트리 노드 포인터
*@param recursive 재귀 옵션 플래그
*@param dir_count 디렉토리 개수 포인터
*@param file_count 파일 개수 포인터
*@return 발견된 엔트리 수
*/
int	process_double_indirect_block(int fd, struct my_ext2_super_block *sb, 
								 struct my_ext2_group_desc *gd,
								 unsigned int double_indirect_block_num, 
								 DirTreeNode *parent_node, int recursive, 
								 int *dir_count, int *file_count)
{
	int entries_found = 0;
	unsigned int block_size = get_block_size(sb);
	unsigned char *block_buf = (unsigned char *)malloc(block_size);
		
	#ifdef DEBUG_TREE
		printf("Processing double indirect block %u\n", double_indirect_block_num);
	#endif

	// 이중 간접 블록 데이터 읽기
	if (read_data_block(fd, sb, double_indirect_block_num, block_buf) < 0) {
		free(block_buf);
		return 0;
	}
		
	// 이중 간접 블록에서 단일 간접 블록 번호 배열 읽기
	unsigned int *indirect_block_ptrs = (unsigned int *)block_buf;
	unsigned int ptrs_per_block = block_size / sizeof(unsigned int);
		
	// 각 단일 간접 블록 번호에 대해 처리
	for (unsigned int i = 0; i < ptrs_per_block; i++) {
		if (indirect_block_ptrs[i] == 0) {
			continue;  // 빈 포인터 건너뛰기
		}
		
		entries_found += process_indirect_block(fd, sb, gd, indirect_block_ptrs[i], 
											  parent_node, recursive, dir_count, file_count);
	}
		
	free(block_buf);
	return entries_found;
}

/**
*삼중 간접 블록 처리 함수
*
*@param fd 파일 디스크립터
*@param sb 슈퍼블록 포인터
*@param gd 그룹 디스크립터 배열 포인터
*@param triple_indirect_block_num 삼중 간접 블록 번호
*@param parent_node 부모 트리 노드 포인터
*@param recursive 재귀 옵션 플래그
*@param dir_count 디렉토리 개수 포인터
*@param file_count 파일 개수 포인터
*@return 발견된 엔트리 수
*/
int	process_triple_indirect_block(int fd, struct my_ext2_super_block *sb, 
								 struct my_ext2_group_desc *gd,
								 unsigned int triple_indirect_block_num, 
								 DirTreeNode *parent_node, int recursive, 
								 int *dir_count, int *file_count)
{
	int entries_found = 0;
	unsigned int block_size = get_block_size(sb);
	unsigned char *block_buf = (unsigned char *)malloc(block_size);
		
	#ifdef DEBUG_TREE
		printf("Processing triple indirect block %u\n", triple_indirect_block_num);
	#endif

	// 삼중 간접 블록 데이터 읽기
	if (read_data_block(fd, sb, triple_indirect_block_num, block_buf) < 0) {
		free(block_buf);
		return 0;
	}
		
	// 삼중 간접 블록에서 이중 간접 블록 번호 배열 읽기
	unsigned int *double_indirect_block_ptrs = (unsigned int *)block_buf;
	unsigned int ptrs_per_block = block_size / sizeof(unsigned int);
		
	// 각 이중 간접 블록 번호에 대해 처리
	for (unsigned int i = 0; i < ptrs_per_block; i++) {
		if (double_indirect_block_ptrs[i] == 0) {
			continue;  // 빈 포인터 건너뛰기
		}
		
		entries_found += process_double_indirect_block(fd, sb, gd, double_indirect_block_ptrs[i], 
													 parent_node, recursive, dir_count, file_count);
	}
		
	free(block_buf);
	return entries_found;
}

/**
 * 디렉토리의 모든 엔트리를 읽어서 트리 구조를 구축하는 함수
 * 직접 블록 및 간접 블록(단일, 이중, 삼중)을 모두 처리
 * 
 * @param fd 파일 디스크립터
 * @param sb 슈퍼블록 포인터
 * @param gd 그룹 디스크립터 포인터
 * @param dir_inode_num 디렉토리 inode 번호
 * @param parent_node 부모 트리 노드 포인터
 * @param recursive 재귀 옵션 플래그
 * @return 발견된 파일 및 디렉토리 수
 */
int	read_directory_entries(int fd, struct my_ext2_super_block *sb, 
						  struct my_ext2_group_desc *gd,
						  unsigned int dir_inode_num, DirTreeNode *parent_node, 
						  int recursive)
{
	struct my_ext2_inode dir_inode;
	int file_count = 0;
	int dir_count = 0;
		
	// 디렉토리 inode 읽기
	if (read_inode(fd, dir_inode_num, sb, gd, &dir_inode) < 0) {
		return -1;
	}
		
	#ifdef DEBUG_TREE
		printf("Processing directory inode %u with %u blocks\n", dir_inode_num, dir_inode.i_blocks);
	#endif

	// 직접 블록 처리 (i_block[0] ~ i_block[11])
	int direct_entries = 0;
	for (int i = 0; i < EXT2_NDIR_BLOCKS; i++) {
		if (dir_inode.i_block[i] == 0) {
			continue;  // 빈 블록 건너뛰기
		}
		
		direct_entries += process_directory_block(fd, sb, gd, dir_inode.i_block[i], 
												parent_node, recursive, &dir_count, &file_count);
	}

	#ifdef DEBUG_TREE
		printf("Found %d entries in direct blocks\n", direct_entries);
	#endif

	// 단일 간접 블록 처리 (i_block[12])
	int single_indirect_entries = 0;
	if (dir_inode.i_block[EXT2_IND_BLOCK] != 0) {
		single_indirect_entries = process_indirect_block(fd, sb, gd, dir_inode.i_block[EXT2_IND_BLOCK], 
													   parent_node, recursive, &dir_count, &file_count);
		#ifdef DEBUG_TREE
			printf("Found %d entries in single indirect block\n", single_indirect_entries);
		#endif
	}
		
	// 이중 간접 블록 처리 (i_block[13])
	int double_indirect_entries = 0;
	if (dir_inode.i_block[EXT2_DIND_BLOCK] != 0) {
		double_indirect_entries = process_double_indirect_block(fd, sb, gd, dir_inode.i_block[EXT2_DIND_BLOCK], 
															  parent_node, recursive, &dir_count, &file_count);
		#ifdef DEBUG_TREE
			printf("Found %d entries in double indirect blocks\n", double_indirect_entries);
		#endif
	}
		
	// 삼중 간접 블록 처리 (i_block[14])
	int triple_indirect_entries = 0;
	if (dir_inode.i_block[EXT2_TIND_BLOCK] != 0) {
		triple_indirect_entries = process_triple_indirect_block(fd, sb, gd, dir_inode.i_block[EXT2_TIND_BLOCK], 
															  parent_node, recursive, &dir_count, &file_count);
		#ifdef DEBUG_TREE
			printf("Found %d entries in triple indirect blocks\n", triple_indirect_entries);
		#endif
	}
		
	#ifdef DEBUG_TREE
		printf("Directory %u total: %d directories, %d files\n", 
		  dir_inode_num, dir_count, file_count);
	#endif

	return dir_count + file_count;
}

/**
 * 트리 노드 출력 함수
 * 
 * @param node 출력할 노드
 * @param depth 노드의 깊이 (들여쓰기 수준)
 * @param options 출력 옵션 (TREE_OPT_R, TREE_OPT_S, TREE_OPT_P)
 * @param is_last 현재 노드가 부모의 마지막 자식인지 여부
 * @param prefix 들여쓰기 및 연결선을 위한 접두사 배열
 */
void	print_tree_node(DirTreeNode* node, int depth, int options, int is_last, char prefix[1024][10])
{
	if (node == NULL) {
		return;
	}
		
	// 들여쓰기와 트리 라인 출력
	for (int i = 0; i < depth; i++) {
		printf("%s", prefix[i]);
	}
		
	// 현재 항목 연결 기호 (마지막이면 ┗, 아니면 ┣)
	printf("%s ", is_last ? "┗" : "┣");
		
	// 옵션에 따른 추가 정보 출력
	if ((options & TREE_OPT_P) || (options & TREE_OPT_S)) {
		printf("[");
		
		// -p 옵션: 권한 정보 출력
		if (options & TREE_OPT_P) {
			// 파일 타입
			if (S_ISDIR(node->file_type)) printf("d");
			else if (S_ISLNK(node->file_type)) printf("l");
			else printf("-");
			
			// 소유자 권한
			printf("%c%c%c", 
				(node->permissions & S_IRUSR) ? 'r' : '-',
				(node->permissions & S_IWUSR) ? 'w' : '-',
				(node->permissions & S_IXUSR) ? 'x' : '-'
			);
			
			// 그룹 권한
			printf("%c%c%c", 
				(node->permissions & S_IRGRP) ? 'r' : '-',
				(node->permissions & S_IWGRP) ? 'w' : '-',
				(node->permissions & S_IXGRP) ? 'x' : '-'
			);
			
			// 기타 사용자 권한
			printf("%c%c%c", 
				(node->permissions & S_IROTH) ? 'r' : '-',
				(node->permissions & S_IWOTH) ? 'w' : '-',
				(node->permissions & S_IXOTH) ? 'x' : '-'
			);
			
			// 권한과 크기 사이 공백
			if (options & TREE_OPT_S) {
				printf(" ");
			}
		}
		
		// -s 옵션: 크기 정보 출력
		if (options & TREE_OPT_S) {
			printf("%u", node->size);
		}
		
		printf("] ");
	}
		
	// 노드 이름 출력
	printf("%s\n", node->name);
		
	// 다음 레벨의 자식 노드들에 대한 접두사 업데이트
	if (is_last) {
		// 마지막 항목이면 다음 레벨은 "  " (빈 공간)
		strcpy(prefix[depth], "  ");
	} else {
		// 마지막 항목이 아니면 다음 레벨은 "┃ " (수직선)
		strcpy(prefix[depth], "┃ ");
	}
		
	// 재귀 옵션이 꺼져 있고 루트 노드가 아니면 자식 노드를 출력하지 않음
	if (!(options & TREE_OPT_R) && depth > 0) {
		// 접두사 원복
		prefix[depth][0] = '\0';
		return;
	}
		
	// 자식 노드 출력
	if (S_ISDIR(node->file_type)) {
		DirTreeNode* child = node->first_child;
		DirTreeNode* next;
		
		// 자식 노드 개수 계산
		int child_count = 0;
		DirTreeNode* temp = child;
		while (temp != NULL) {
			child_count++;
			temp = temp->next_sibling;
		}
		
		// 각 자식 노드 처리
		int current = 0;
		while (child != NULL) {
			current++;
			next = child->next_sibling;
			print_tree_node(child, depth + 1, options, (current == child_count), prefix);
			child = next;
		}
	}
		
	// 접두사 원복 (함수를 빠져나갈 때)
	prefix[depth][0] = '\0';
}

/**
*트리 노드 메모리 해제 함수
*
*@param node 메모리를 해제할 트리 노드 포인터
*/
void	free_tree_node(DirTreeNode* node)
{
	if (node == NULL) {
		return;
	}
		
	// 자식 노드 메모리 해제
	DirTreeNode* child = node->first_child;
	while (child != NULL) {
		DirTreeNode* next = child->next_sibling;
		free_tree_node(child);
		child = next;
	}
		
	// 자신의 메모리 해제
	free(node);
}

/**
 * 트리 구조 출력을 위한 주 함수
 *
 * @param cmd 명령어 구조체 포인터
 */
void	tree(Command *cmd)
{
	int fd = open(img_path, O_RDONLY);
		
	struct my_ext2_super_block sb;
	if (read_super_block(fd, &sb) < 0) {
		#ifdef DEBUG_TREE
			fprintf(stderr, "Error reading superblock\n");
		#endif
		close(fd);
		return;
	}
		
	// 블록 그룹 디스크립터 읽기
	int block_size = get_block_size(&sb);
	int block_groups_count = (sb.s_blocks_count + sb.s_blocks_per_group - 1) 
							/ sb.s_blocks_per_group;
		
	int gdt_size = block_groups_count * sizeof(struct my_ext2_group_desc);
	struct my_ext2_group_desc *gd = (struct my_ext2_group_desc *)malloc(gdt_size);
		
	off_t gdt_offset = (sb.s_first_data_block + 1) * block_size;
	lseek(fd, gdt_offset, SEEK_SET);
		
	read(fd, gd, gdt_size);
		
	// 경로의 inode 번호 찾기
	unsigned int inode_num = path_to_inode(fd, &sb, gd, cmd->path);
	if (inode_num == 0) {
		help_all();
		free(gd);
		close(fd);
		return;
	}
		
	// inode 정보 읽기
	struct my_ext2_inode inode;
	if (read_inode(fd, inode_num, &sb, gd, &inode) < 0) {
		#ifdef DEBUG_TREE
			fprintf(stderr, "Error: Failed to read inode\n");
		#endif
		free(gd);
		close(fd);
		return;
	}
		
	// 디렉토리 확인
	if (!S_ISDIR(inode.i_mode)) {
		fprintf(stdout, "Error: '%s' is not directory\n", cmd->path);
		free(gd);
		close(fd);
		return;
	}
		
	// 루트 노드 생성
	char* root_name = cmd->path;
	// 경로가 "."인 경우 표시할 이름으로 "." 사용
	if (strcmp(cmd->path, ".") == 0) {
		root_name = ".";
	}
		
	DirTreeNode* root = create_tree_node(root_name, inode_num, inode.i_mode, inode.i_size, inode.i_mode & 0xFFF);
	if (root == NULL) {
		#ifdef DEBUG_TREE
			fprintf(stderr, "Error: Failed to create root tree node\n");
		#endif
		free(gd);
		close(fd);
		return;
	}
		
	// 디렉토리 내용 읽기
	read_directory_entries(fd, &sb, gd, inode_num, root, cmd->options & TREE_OPT_R);
		
	 // 루트 경로 출력 (옵션에 따라 추가 정보 포함)
	if ((cmd->options & TREE_OPT_P) || (cmd->options & TREE_OPT_S)) {
		printf("[");
		
		// -p 옵션: 권한 정보 출력
		if (cmd->options & TREE_OPT_P) {
			// 파일 타입
			if (S_ISDIR(root->file_type)) printf("d");
			else if (S_ISLNK(root->file_type)) printf("l");
			else printf("-");
			
			// 소유자 권한
			printf("%c%c%c", 
				(root->permissions & S_IRUSR) ? 'r' : '-',
				(root->permissions & S_IWUSR) ? 'w' : '-',
				(root->permissions & S_IXUSR) ? 'x' : '-'
			);
			
			// 그룹 권한
			printf("%c%c%c", 
				(root->permissions & S_IRGRP) ? 'r' : '-',
				(root->permissions & S_IWGRP) ? 'w' : '-',
				(root->permissions & S_IXGRP) ? 'x' : '-'
			);
			
			// 기타 사용자 권한
			printf("%c%c%c", 
				(root->permissions & S_IROTH) ? 'r' : '-',
				(root->permissions & S_IWOTH) ? 'w' : '-',
				(root->permissions & S_IXOTH) ? 'x' : '-'
			);
			
			// 권한과 크기 사이 공백
			if (cmd->options & TREE_OPT_S) {
				printf(" ");
			}
		}
		
		// -s 옵션: 크기 정보 출력
		if (cmd->options & TREE_OPT_S) {
			printf("%u", root->size);
		}
		
		printf("] ");
	}
		
	printf("%s\n", root_name);
		
	// 트리 접두사 배열 초기화
	char prefix[1024][10] = {{0}};
		
	// 자식 노드 출력
	DirTreeNode* child = root->first_child;
	DirTreeNode* next;
		
	// 자식 노드 개수 계산
	int child_count = 0;
	DirTreeNode* temp = child;
	while (temp != NULL) {
		child_count++;
		temp = temp->next_sibling;
	}
		
	// 각 자식 노드 처리
	int current = 0;
	while (child != NULL) {
		current++;
		next = child->next_sibling;
		print_tree_node(child, 0, cmd->options, (current == child_count), prefix);
		child = next;
	}
		
	// 파일과 디렉토리 개수 계산
	int file_count = 0;
	int dir_count = 0;
	count_files_and_dirs(root, &file_count, &dir_count);
		
	// 결과 출력
	printf("\n%d directories, %d files\n\n", dir_count + 1, file_count);
		
	// 메모리 해제
	free_tree_node(root);
	free(gd);
	close(fd);
}

/**
 * 트리 내 파일과 디렉토리 개수 계산 함수
 * 
 * @param node 개수를 세기 시작할 트리 노드 포인터
 * @param file_count 파일 개수를 저장할 포인터
 * @param dir_count 디렉토리 개수를 저장할 포인터
 */
void	count_files_and_dirs(DirTreeNode* node, int* file_count, int* dir_count)
{
	if (node == NULL) {
		return;
	}
		
	DirTreeNode* child = node->first_child;
	while (child != NULL) {
		if (S_ISDIR(child->file_type)) {
			(*dir_count)++;
		} else {
			(*file_count)++;
		}
		
		// 자식 노드의 자식들도 재귀적으로 세기
		count_files_and_dirs(child, file_count, dir_count);
		
		child = child->next_sibling;
	}
}
