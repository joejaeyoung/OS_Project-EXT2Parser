#include "ssu_ext2.h"

/**
 * 파일 내용 출력 함수
 * 
 * @param fd 파일 디스크립터
 * @param sb 슈퍼블록 포인터
 * @param inode 파일의 inode 구조체 포인터
 * @param line_count 출력할 라인 수 (0 또는 음수면 전체 출력)
 * @return 성공 시 0, 실패 시 음수 값
 */
int	print_file_content(int fd, struct my_ext2_super_block *sb, 
					  struct my_ext2_inode *inode, 
					  int line_count)
{
	unsigned int block_size = get_block_size(sb);
	unsigned char *buffer = (unsigned char *)malloc(block_size);
		
	int total_printed = 0;
	int line_printed = 0;
		
	// 직접 블록 처리 (i_block[0] ~ i_block[11])
	for (int i = 0; i < EXT2_NDIR_BLOCKS; i++) {
		if (inode->i_block[i] == 0 || 
			total_printed >= inode->i_size ||
			(line_count > 0 && line_printed >= line_count)) {
			break;
		}
		
		if (read_data_block(fd, sb, inode->i_block[i], buffer) < 0) {
			#ifdef DEBUG_PRINT
				fprintf(stderr, "Failed to read data block %u\n", inode->i_block[i]);
			#endif
			continue;
		}
		
		// 이 블록에서 출력할 바이트 수 계산
		unsigned int bytes_to_print = 
			(total_printed + block_size > inode->i_size) ? 
			(inode->i_size - total_printed) : block_size;
		
		if (line_count <= 0) {
			write(STDOUT_FILENO, buffer, bytes_to_print);
			total_printed += bytes_to_print;
		} else {
			for (unsigned int j = 0; j < bytes_to_print && line_printed < line_count; j++) {
				putchar(buffer[j]);
				total_printed++;
				
				if (buffer[j] == '\n') {
					line_printed++;
					if (line_printed >= line_count) {
						break;
					}
				}
			}
		}
	}
		
	// 간접 블록이 필요한 경우 처리
	if (total_printed < inode->i_size && 
		(line_count <= 0 || line_printed < line_count) &&
		inode->i_block[EXT2_IND_BLOCK] != 0) {
		
		// 간접 블록 데이터 읽기
		unsigned int *indirect_blocks = (unsigned int *)malloc(block_size);
		
		if (read_data_block(fd, sb, inode->i_block[EXT2_IND_BLOCK], (unsigned char *)indirect_blocks) < 0) {
			#ifdef DEBUG_PRINT
				fprintf(stderr, "Failed to read indirect block %u\n", inode->i_block[EXT2_IND_BLOCK]);
			#endif
			free(indirect_blocks);
			free(buffer);
			return -1;
		}
		
		// 간접 블록의 각 블록 번호에 대해 처리
		unsigned int ptrs_per_block = block_size / sizeof(unsigned int);
		for (unsigned int i = 0; i < ptrs_per_block; i++) {
			if (indirect_blocks[i] == 0 || 
				total_printed >= inode->i_size ||
				(line_count > 0 && line_printed >= line_count)) {
				break;
			}
			
			if (read_data_block(fd, sb, indirect_blocks[i], buffer) < 0) {
				#ifdef DEBUG_PRINT
					fprintf(stderr, "Failed to read data block %u\n", indirect_blocks[i]);
				#endif
				continue;
			}
			
			// 이 블록에서 출력할 바이트 수 계산
			unsigned int bytes_to_print = 
				(total_printed + block_size > inode->i_size) ? 
				(inode->i_size - total_printed) : block_size;
			
			if (line_count <= 0) {
				write(STDOUT_FILENO, buffer, bytes_to_print);
				total_printed += bytes_to_print;
			} else {
				for (unsigned int j = 0; j < bytes_to_print && line_printed < line_count; j++) {
					putchar(buffer[j]);
					total_printed++;
					
					if (buffer[j] == '\n') {
						line_printed++;
						if (line_printed >= line_count) {
							break;
						}
					}
				}
			}
		}
		
		free(indirect_blocks);
	}
		
	// 이중 간접 블록 처리
	if (total_printed < inode->i_size && 
		(line_count <= 0 || line_printed < line_count) &&
		inode->i_block[EXT2_DIND_BLOCK] != 0) {
		
		// 이중 간접 블록 읽기
		unsigned int *dind_blocks = (unsigned int *)malloc(block_size);
		
		if (read_data_block(fd, sb, inode->i_block[EXT2_DIND_BLOCK], (unsigned char *)dind_blocks) < 0) {
			#ifdef DEBUG_PRINT
				fprintf(stderr, "Failed to read double indirect block %u\n", inode->i_block[EXT2_DIND_BLOCK]);
			#endif
			free(dind_blocks);
			free(buffer);
			return -1;
		}
		
		// 이중 간접 블록 내의 각 간접 블록 처리
		unsigned int ptrs_per_block = block_size / sizeof(unsigned int);
		for (unsigned int i = 0; i < ptrs_per_block; i++) {
			if (dind_blocks[i] == 0 || 
				total_printed >= inode->i_size ||
				(line_count > 0 && line_printed >= line_count)) {
				break;
			}
			
			unsigned int *indirect_blocks = (unsigned int *)malloc(block_size);
			
			if (read_data_block(fd, sb, dind_blocks[i], (unsigned char *)indirect_blocks) < 0) {
				#ifdef DEBUG_PRINT
					fprintf(stderr, "Failed to read indirect block %u\n", dind_blocks[i]);
				#endif
				free(indirect_blocks);
				continue;
			}
			
			// 간접 블록 내의 각 데이터 블록 처리
			for (unsigned int j = 0; j < ptrs_per_block; j++) {
				if (indirect_blocks[j] == 0 || 
					total_printed >= inode->i_size ||
					(line_count > 0 && line_printed >= line_count)) {
					break;
				}
				
				if (read_data_block(fd, sb, indirect_blocks[j], buffer) < 0) {
					#ifdef DEBUG_PRINT
						fprintf(stderr, "Failed to read data block %u\n", indirect_blocks[j]);
					#endif
					continue;
				}
				
				unsigned int bytes_to_print = 
					(total_printed + block_size > inode->i_size) ? 
					(inode->i_size - total_printed) : block_size;
				
				if (line_count <= 0) {
					write(STDOUT_FILENO, buffer, bytes_to_print);
					total_printed += bytes_to_print;
				} else {
					for (unsigned int k = 0; k < bytes_to_print && line_printed < line_count; k++) {
						putchar(buffer[k]);
						total_printed++;
						
						if (buffer[k] == '\n') {
							line_printed++;
							if (line_printed >= line_count) {
								break;
							}
						}
					}
				}
			}
			
			free(indirect_blocks);
		}
		
		free(dind_blocks);
	}
		
	// 삼중 간접 블록 처리
	if (total_printed < inode->i_size && 
		(line_count <= 0 || line_printed < line_count) &&
		inode->i_block[EXT2_TIND_BLOCK] != 0) {
		
		// 삼중 간접 블록 읽기
		unsigned int *tind_blocks = (unsigned int *)malloc(block_size);
		
		if (read_data_block(fd, sb, inode->i_block[EXT2_TIND_BLOCK], (unsigned char *)tind_blocks) < 0) {
			#ifdef DEBUG_PRINT
				fprintf(stderr, "Failed to read triple indirect block %u\n", inode->i_block[EXT2_TIND_BLOCK]);
			#endif
			free(tind_blocks);
			free(buffer);
			return -1;
		}
		
		// 삼중 간접 블록 내의 각 이중 간접 블록 처리
		unsigned int ptrs_per_block = block_size / sizeof(unsigned int);
		for (unsigned int i = 0; i < ptrs_per_block; i++) {
			if (tind_blocks[i] == 0 || 
				total_printed >= inode->i_size ||
				(line_count > 0 && line_printed >= line_count)) {
				break;
			}
			
			// 이중 간접 블록 읽기
			unsigned int *dind_blocks = (unsigned int *)malloc(block_size);
			
			if (read_data_block(fd, sb, tind_blocks[i], (unsigned char *)dind_blocks) < 0) {
				#ifdef DEBUG_PRINT
					fprintf(stderr, "Failed to read double indirect block %u\n", tind_blocks[i]);
				#endif
				free(dind_blocks);
				continue;
			}
			
			// 이중 간접 블록 내의 각 간접 블록 처리
			for (unsigned int j = 0; j < ptrs_per_block; j++) {
				if (dind_blocks[j] == 0 || 
					total_printed >= inode->i_size ||
					(line_count > 0 && line_printed >= line_count)) {
					break;
				}
				
				// 간접 블록 읽기
				unsigned int *indirect_blocks = (unsigned int *)malloc(block_size);
				
				if (read_data_block(fd, sb, dind_blocks[j], (unsigned char *)indirect_blocks) < 0) {
					#ifdef DEBUG_PRINT
						fprintf(stderr, "Failed to read indirect block %u\n", dind_blocks[j]);
					#endif
					free(indirect_blocks);
					continue;
				}
				
				// 간접 블록 내의 각 데이터 블록 처리
				for (unsigned int k = 0; k < ptrs_per_block; k++) {
					if (indirect_blocks[k] == 0 || 
						total_printed >= inode->i_size ||
						(line_count > 0 && line_printed >= line_count)) {
						break;
					}
					
					// 데이터 블록 읽기
					if (read_data_block(fd, sb, indirect_blocks[k], buffer) < 0) {
						#ifdef DEBUG_PRINT
							fprintf(stderr, "Failed to read data block %u\n", indirect_blocks[k]);
						#endif
						continue;
					}
					
					// 이 블록에서 출력할 바이트 수 계산
					unsigned int bytes_to_print = 
						(total_printed + block_size > inode->i_size) ? 
						(inode->i_size - total_printed) : block_size;
					
					if (line_count <= 0) {
						// 라인 수 제한 없이 모든 내용 출력
						write(STDOUT_FILENO, buffer, bytes_to_print);
						total_printed += bytes_to_print;
					} else {
						// 특정 라인 수만 출력
						for (unsigned int m = 0; m < bytes_to_print && line_printed < line_count; m++) {
							putchar(buffer[m]);
							total_printed++;
							
							if (buffer[m] == '\n') {
								line_printed++;
								if (line_printed >= line_count) {
									break;
								}
							}
						}
					}
				}
				
				free(indirect_blocks);
			}
			
			free(dind_blocks);
		}
		
		free(tind_blocks);
	}
		
	free(buffer);
	return 0;
}

/**
 * print 명령어 구현 함수
 * 
 * @param cmd 명령어 구조체 포인터
 */
void	print(Command *cmd)
{
	int fd = open(img_path, O_RDONLY);
		
	struct my_ext2_super_block sb;
	if (read_super_block(fd, &sb) < 0) {
		#ifdef DEBUG_PRINT
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
	if (!gd) {
		#ifdef DEBUG_PRINT
			fprintf(stderr, "Error allocating memory for group descriptors\n");
		#endif
		close(fd);
		return;
	}
		
	off_t gdt_offset = (sb.s_first_data_block + 1) * block_size;
	if (lseek(fd, gdt_offset, SEEK_SET) != gdt_offset) {
		#ifdef DEBUG_PRINT
			fprintf(stderr, "Error seeking to group descriptor table\n");
		#endif
		free(gd);
		close(fd);
		return;
	}
		
	if (read(fd, gd, gdt_size) != gdt_size) {
		#ifdef DEBUG_PRINT
			fprintf(stderr, "Error reading group descriptor table\n");
		#endif
		free(gd);
		close(fd);
		return;
	}
		
	// 루트 디렉토리부터 시작하는 디렉토리 트리 구축
	DirTreeNode *root = create_tree_node(".", EXT2_ROOT_INO, S_IFDIR, 0, 0755);
	if (!root) {
		#ifdef DEBUG_PRINT
			fprintf(stderr, "Error creating directory tree\n");
		#endif
		free(gd);
		close(fd);
		return;
	}
		
	// 루트 디렉토리 inode 정보 읽기
	struct my_ext2_inode root_inode;
	if (read_inode(fd, EXT2_ROOT_INO, &sb, gd, &root_inode) < 0) {
		#ifdef DEBUG_PRINT
			fprintf(stderr, "Error reading root directory inode\n");
		#endif
		free_tree_node(root);
		free(gd);
		close(fd);
		return;
	}
		
	//디렉토리 트리 구축 (재귀적으로)
	read_directory_entries(fd, &sb, gd, EXT2_ROOT_INO, root, 1); // 1은 재귀적으로 구축
		
	char path_copy[MAX_PATH];
	strncpy(path_copy, cmd->path, MAX_PATH - 1);
	path_copy[MAX_PATH - 1] = '\0';
		
	// 경로 끝의 슬래시 제거
	size_t path_len = strlen(path_copy);
	if (path_len > 1 && path_copy[path_len - 1] == '/') {
		path_copy[path_len - 1] = '\0';
	}
		
	// 경로 탐색을 위한 노드 스택 (최대 깊이를 1024으로 가정)
	DirTreeNode *node_stack[1024];
	int stack_top = 0;
		
	// 시작 노드 설정 (절대 경로는 루트부터, 상대 경로는 현재 디렉토리부터)
	DirTreeNode *current = root;
	node_stack[stack_top++] = current; // 루트 노드 스택에 푸시
		
	// 경로 정규화
	char *path_ptr = path_copy;
		
	// 절대 경로인 경우
	if (path_copy[0] == '/') {
		path_ptr++; // '/' 건너뛰기
	} 
	// "./"로 시작하는 경우 (현재 디렉토리)
	else if (strncmp(path_copy, "./", 2) == 0) {
		path_ptr += 2; // "./" 건너뛰기
	}
		
	// 경로 토큰 먼저 추출 (처리 전에 모든 토큰 확인)
	char *tokens[MAX_PATH] = {0};
	int token_count = 0;
	char *saveptr;
	char *token = strtok_r(path_ptr, "/", &saveptr);
		
	while (token != NULL && token_count < MAX_PATH) {
		tokens[token_count++] = token;
		token = strtok_r(NULL, "/", &saveptr);
	}
		
	// 토큰 처리
	for (int i = 0; i < token_count; i++) {
		token = tokens[i];
		
		// "." 토큰은 현재 디렉토리를 의미하므로 무시
		if (strcmp(token, ".") == 0) {
			continue;
		}
		
		// ".." 토큰은 상위 디렉토리를 의미
		if (strcmp(token, "..") == 0) {
			// 스택이 비어있지 않고, 현재 노드가 루트가 아닌 경우에만 상위로 이동
			if (stack_top > 1) {
				stack_top--; // 현재 노드를 스택에서 팝
				current = node_stack[stack_top - 1]; // 이전 노드가 현재 노드가 됨
			}
			// 루트 디렉토리에서의 ".."는 여전히 루트를 가리킴
			continue;
		}
		
		// 마지막 토큰이 아니라면 현재 노드가 디렉토리인지 확인
		if (i < token_count - 1 && !S_ISDIR(current->file_type)) {
			#ifdef DEBUG_PRINT
				fprintf(stderr, "Error: '%s' is not a directory in the path\n", current->name);
			#endif
			help_all();
			free_tree_node(root);
			free(gd);
			close(fd);
			return;
		}
		
		int found = 0;
		
		// 현재 디렉토리의 자식 노드 중에서 찾기
		DirTreeNode *child = current->first_child;
		while (child != NULL) {
			if (strcmp(child->name, token) == 0) {
				current = child;
				node_stack[stack_top++] = current; // 새 노드를 스택에 푸시
				found = 1;
				break;
			}
			child = child->next_sibling;
		}
		
		if (!found) {
			#ifdef DEBUG_PRINT
				fprintf(stderr, "Error: '%s' not found in '%s'\n", token, path_copy);
			#endif
			help_all();
			free_tree_node(root);
			free(gd);
			close(fd);
			return;
		}
		
		// 마지막 토큰이 아니라면 찾은 노드가 디렉토리인지 확인
		if (i < token_count - 1 && !S_ISDIR(current->file_type)) {
			#ifdef DEBUG_PRINT
				fprintf(stderr, "Error: '%s' is not a directory\n", token);
			#endif
			help_all();
			free_tree_node(root);
			free(gd);
			close(fd);
			return;
		}
	}
		
	// 파일인지 확인
	if (S_ISDIR(current->file_type)) {
		fprintf(stdout, "Error: '%s' is not file\n", cmd->path);
		free_tree_node(root);
		free(gd);
		close(fd);
		return;
	}
		
	// 파일 inode 정보 읽기
	struct my_ext2_inode file_inode;
	if (read_inode(fd, current->inode_num, &sb, gd, &file_inode) < 0) {
		#ifdef DEBUG_PRINT
			fprintf(stderr, "Error: Failed to read file inode\n");
		#endif
		free_tree_node(root);
		free(gd);
		close(fd);
		return;
	}
		
	// 파일 내용 출력
	print_file_content(fd, &sb, &file_inode, cmd->extra_param);
		
	// 메모리 해제
	free_tree_node(root);
	free(gd);
	close(fd);
}

// /**
//  * inode 번호로 트리에서 노드 찾기
//  * 
//  * @param node 검색 시작 노드
//  * @param inode_num 찾을 inode 번호
//  * @return 찾은 노드 포인터, 없으면 NULL
//  */
// DirTreeNode*	find_node_by_inode(DirTreeNode* node, unsigned int inode_num)
// {
// 	if (!node) return NULL;
		
// 	if (node->inode_num == inode_num) {
// 		return node;
// 	}
		
// 	// 자식 노드에서 찾기
// 	DirTreeNode* child = node->first_child;
// 	while (child) {
// 		DirTreeNode* found = find_node_by_inode(child, inode_num);
// 		if (found) return found;
// 		child = child->next_sibling;
// 	}
		
// 	return NULL;
// }
