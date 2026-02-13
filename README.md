<div align="center">

# 📂 EXT2 File System Parser

**EXT2 파일 시스템 이미지를 분석하여 디렉토리 구조 탐색 및 파일 내용 출력을 제공하는 CLI 프로그램**

[![Hits](https://hits.sh/github.com/joejaeyoung/OS_Project-EXT2Parser.svg)](https://github.com/joejaeyoung/OS_Project-EXT2Parser)

</div>

---

## 📋 프로젝트 정보

|    항목     | 내용           |
| :-------: | :----------- |
|  **분야**   | OS (파일 시스템)  |
| **개발 기간** | 2025.05      |
| **개발 환경** | Ubuntu Linux |
|  **언어**   | C            |

---

## 📖 프로젝트 소개

이 프로젝트는 **EXT2 파일 시스템 이미지 파일(.img)** 을 분석하는 CLI 프로그램 `ssu_ext2`입니다.

EXT2 파일 시스템의 **슈퍼블록, 블록 그룹 디스크립터, inode, 디렉토리 엔트리** 등 내부 구조를 직접 파싱하여, 루트 디렉토리부터 하위 경로 및 파일을 탐색합니다. **링크드 리스트 기반 트리 구조**로 디렉토리 계층을 표현하며, **직접 블록뿐만 아니라 단일/이중/삼중 간접 블록**까지 모두 처리하여 대용량 파일도 정확히 다룹니다.

### 제공 기능

- `tree` — 디렉토리 구조를 트리 형태로 출력 (재귀, 크기, 권한 옵션 지원)
- `print` — 파일 내용 출력 (라인 수 제한 옵션 지원)
- `help` — 명령어별 상세 도움말 출력
- `exit` — 프로그램 종료

---

## 🚀 시작 가이드

### Requirements

- GCC
- GNU Make

### Installation & Build

```bash
# 1. 소스 클론
$ git clone https://github.com/jojaeyoung/OS_Project-EXT2Parser.git
$ cd OS_Project-EXT2Parser/srcs

# 2. 빌드
$ make

# 3. 실행 (EXT2 이미지 파일 경로 지정)
$ ./ssu_ext2 <ext2_image_file.img>
```

### 실행 예시

```bash
$ ./ssu_ext2 ext2disk.img
$> tree / -r -s -p
$> print /dir1/file1.txt -n 10
$> help tree
20201505> exit
```

---

## 🛠️ Stacks

### Environment

![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)

### Development

![C](https://img.shields.io/badge/C-A8B9CC?style=for-the-badge&logo=c&logoColor=white)
![GCC](https://img.shields.io/badge/GCC-333333?style=for-the-badge&logo=gnu&logoColor=white)

### Config

![Makefile](https://img.shields.io/badge/Makefile-064F8C?style=for-the-badge&logo=gnu&logoColor=white)

---

## 📺 명령어 인터페이스

### `tree <PATH> [OPTION]...`

| 항목 | 설명 |
|:---|:---|
| **역할** | 지정된 경로의 디렉토리 구조를 트리 형태로 출력 |
| **PATH** | 접근 가능한 디렉토리 경로 (절대/상대 경로 지원) |
| **-r 옵션** | 재귀적으로 모든 하위 디렉토리까지 출력 |
| **-s 옵션** | 각 파일/디렉토리의 크기를 함께 출력 |
| **-p 옵션** | 각 파일/디렉토리의 권한 정보를 함께 출력 |
| **출력 제외** | `.`, `..`, `lost+found` 디렉토리는 출력에서 제외 |

#### 사용 예시

```bash
# 루트 디렉토리의 첫 번째 레벨만 출력
tree /

# 재귀 + 크기 + 권한 모두 표시
tree / -r -s -p

# 하위 디렉토리 재귀 출력
tree /dir1 -r
```

### `print <PATH> [OPTION]...`

| 항목 | 설명 |
|:---|:---|
| **역할** | 지정된 경로의 파일 내용을 화면에 출력 |
| **PATH** | 접근 가능한 파일 경로 (절대/상대 경로 지원) |
| **-n \<line\>** | 파일의 처음부터 지정한 라인 수만큼만 출력 |

#### 사용 예시

```bash
# 파일 전체 출력
print /dir1/file1.txt

# 상위 10줄만 출력
print /dir1/file1.txt -n 10
```

### `help [COMMAND]`

| 항목 | 설명 |
|:---|:---|
| **역할** | 명령어별 상세 도움말 출력 |
| **COMMAND** | `tree`, `print`, `help`, `exit` 중 하나 (생략 시 전체 요약) |

### `exit`

| 항목 | 설명 |
|:---|:---|
| **역할** | `ssu_ext2` 프로그램 종료 |

---

## ⭐ 주요 기능

### 1. EXT2 파일 시스템 파싱

- **슈퍼블록** 읽기 및 매직 넘버(`0xEF53`) 검증
- **블록 그룹 디스크립터** 테이블 파싱
- **inode** 구조체를 통한 파일/디렉토리 메타데이터 접근
- 경로 문자열 → inode 번호 변환 (`path_to_inode`)

### 2. 디렉토리 트리 구조 (링크드 리스트)

- `DirTreeNode` 구조체 기반의 **자식-형제 링크드 리스트** 트리
- `create_tree_node()` / `add_child_node()` 로 동적 트리 구축
- `read_directory_entries()` 로 디렉토리 엔트리 자동 탐색
- `free_tree_node()` 로 메모리 해제

### 3. 간접 블록 완전 지원

- **직접 블록** (12개) 처리
- **단일 간접 블록** (`process_indirect_block`)
- **이중 간접 블록** (`process_double_indirect_block`)
- **삼중 간접 블록** (`process_triple_indirect_block`)
- 대용량 파일 및 대규모 디렉토리도 정확히 처리

### 4. 경로 처리

- **절대 경로** (`/dir1/file.txt`) 지원
- **상대 경로** (`./dir1`, `../dir2`) 지원
- 경로 유효성 검사 및 타입 확인 (디렉토리/파일 구분)

### 5. 오류 처리

- 유효하지 않은 EXT2 이미지: `"Error: bad file system"`
- 디렉토리가 아닌 경로에 tree 사용: `"Error: '<PATH>' is not directory"`
- 파일이 아닌 경로에 print 사용: `"Error: '<PATH>' is not file"`
- 존재하지 않는 경로: 도움말(usage) 출력
- 디렉토리 깊이 최대 **1024 레벨** 제한

---

## 🏗️ 아키텍처

### 프로그램 동작 흐름

```
사용자 입력
    │
    ▼
┌─────────────────────────────┐
│  main()                     │
│  ├─ check_super_magic()     │  ← EXT2 매직 넘버 검증
│  ├─ get_input_line()        │  ← 사용자 명령어 입력
│  │                          │
│  ├─ "help" → help()         │
│  ├─ "exit" → 프로그램 종료    │
│  ├─ "tree" → parse → tree() │
│  └─ "print"→ parse → print()│
└─────────────────────────────┘
```

### tree 명령어 콜 그래프

```
tree()
├── create_tree_node()
├── read_directory_entries()
│   ├── read_inode()
│   ├── process_directory_block()
│   │   ├── read_data_block()
│   │   ├── create_tree_node()
│   │   ├── add_child_node()
│   │   └── read_directory_entries()  ← 재귀 호출
│   ├── process_indirect_block()
│   ├── process_double_indirect_block()
│   └── process_triple_indirect_block()
├── print_tree_node()
├── count_files_and_dirs()
└── free_tree_node()
```

### print 명령어 콜 그래프

```
print()
├── create_tree_node()
├── read_inode()
├── read_directory_entries()
├── print_file_content()
│   ├── get_block_size()
│   └── read_data_block()  ← 직접/단일/이중/삼중 간접 블록 처리
└── free_tree_node()
```

### EXT2 핵심 상수

| 상수 | 값 | 설명 |
|:---|:---:|:---|
| `EXT2_SUPER_MAGIC` | 0xEF53 | 슈퍼블록 매직 넘버 |
| `EXT2_ROOT_INO` | 2 | 루트 디렉토리 inode 번호 |
| `EXT2_NDIR_BLOCKS` | 12 | 직접 블록 개수 |
| `EXT2_IND_BLOCK` | 12 | 단일 간접 블록 인덱스 |
| `EXT2_DIND_BLOCK` | 13 | 이중 간접 블록 인덱스 |
| `EXT2_TIND_BLOCK` | 14 | 삼중 간접 블록 인덱스 |
| `EXT2_N_BLOCKS` | 15 | i_block 배열 크기 |

### 주요 구조체

#### `Command` (명령어 파싱 결과)

| 필드 | 타입 | 설명 |
|:---|:---:|:---|
| `cmd_type` | char[10] | 사용자가 입력한 명령어 |
| `path` | char[4096] | 사용자가 입력한 경로 |
| `options` | int | tree 명령어 옵션 플래그 (`TREE_OPT_R`, `TREE_OPT_S`, `TREE_OPT_P`) |
| `extra_param` | int | print 명령어의 `-n` 옵션 값 |

#### `DirTreeNode` (디렉토리 트리 노드)

| 필드 | 타입 | 설명 |
|:---|:---:|:---|
| `name` | char[256] | 파일/디렉토리 이름 |
| `inode_num` | int | inode 번호 |
| `file_type` | int | 파일 타입 (`S_IFDIR`, `S_IFREG` 등) |
| `size` | uint | 파일 크기 |
| `permissions` | uint | 권한 정보 |
| `first_child` | DirTreeNode* | 첫 번째 자식 노드 포인터 |
| `next_sibling` | DirTreeNode* | 다음 형제 노드 포인터 |

### 디렉토리 구조

```
OS_Project-EXT2Parser/
├── README.md
└── srcs/
    ├── Makefile            # 빌드 설정
    ├── ext2.h              # EXT2 구조체 정의 (슈퍼블록, inode, 디렉토리 엔트리, 그룹 디스크립터)
    ├── ssu_ext2.h          # 프로젝트 헤더 (Command, DirTreeNode 구조체 + 함수 프로토타입)
    ├── ssu_ext2.c          # main 함수 (명령어 루프, 슈퍼블록 검증)
    ├── tree.c              # tree 명령어 구현 (트리 구축, 출력, 간접 블록 처리)
    ├── print.c             # print 명령어 구현 (파일 내용 출력, 간접 블록 처리)
    ├── parse.c             # 명령어 파싱 (tree/print 옵션 처리)
    ├── validate.c          # 경로 유효성 검사
    ├── help.c              # 도움말 출력
    ├── ext2_utils.c        # EXT2 유틸리티 (슈퍼블록 읽기, 블록 크기 계산, 데이터 블록 읽기)
    ├── ext2_inode.c        # inode 관련 (path_to_inode, read_inode, find_entry_in_dir)
    ├── utils_split.c       # 문자열 분리 유틸리티 (fix_split)
    └── debug.c             # 디버깅 출력
```

### 파일 역할 관계

| 파일 | 역할 | 핵심 내용 |
|:---|:---|:---|
| `ext2.h` | 데이터 구조 | EXT2 슈퍼블록, inode, 디렉토리 엔트리, 그룹 디스크립터 구조체 |
| `ssu_ext2.h` | 프로젝트 헤더 | Command, DirTreeNode 구조체 + 전체 함수 프로토타입 |
| `ssu_ext2.c` | 메인 로직 | 명령어 입력 루프, 매직 넘버 검증, 명령어 분기 |
| `tree.c` | 트리 출력 | 트리 구축/출력, 직접·간접 블록 처리, 파일/디렉토리 카운트 |
| `print.c` | 파일 출력 | 파일 내용 읽기/출력, 직접·간접 블록 처리 |
| `parse.c` | 명령어 파싱 | tree/print 명령어 옵션 파싱 및 검증 |
| `validate.c` | 경로 검증 | 경로 유효성·타입 검사 |
| `ext2_utils.c` | EXT2 유틸 | 슈퍼블록 읽기, 블록 크기 계산, 데이터 블록 읽기 |
| `ext2_inode.c` | inode 처리 | 경로→inode 변환, inode 읽기, 디렉토리 엔트리 검색 |
| `help.c` | 도움말 | 명령어별 usage 출력 |
| `utils_split.c` | 문자열 유틸 | 구분자 기반 문자열 분리 |
| `debug.c` | 디버깅 | 명령어 파싱 결과, 디렉토리 블록 디버깅 출력 |
