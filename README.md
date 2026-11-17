# MP3-Tag-Reader-and-Editor-in-C
A C-based MP3 Tag Reader and Editor that uses ID3v2.3 to view and modify audio metadata through binary file handling.
# MP3 Tag Reader and Editor in C

## 📌 Project Overview

The MP3 Tag Reader and Editor is a command-line application developed using C programming. It allows users to read and modify metadata (ID3 tags) of MP3 audio files.

The application supports viewing MP3 metadata and editing selected tags without modifying the actual audio data.

## 🚀 Features

- Read and display MP3 metadata.
- Edit existing MP3 metadata.
- Support for ID3v2.3 format.
- Update individual metadata frames.
- Preserve unchanged metadata frames.
- Copy original audio data into the updated MP3 file.
- Dynamic memory allocation for frame data.
- File handling using binary read and write operations.

## 🛠️ Technologies Used

- Programming Language: C
- Compiler: GCC
- Operating System: Linux
- Concepts: Pointers, Structures, Dynamic Memory Allocation, File Handling, Command-Line Arguments, String Handling, and Endian Conversion.

## 📂 Project Structure

```text
MP3_Tag_Reader/
│
├── mp3_main.c
├── mp3_header.h
├── functions.c
├── mp3_view.c
├── mp3_edit.c
├── sample.mp3
└── README.md
```

## ⚙️ Compilation

Compile the project using GCC:

```bash
gcc *.c -o mp3reader
```

## ▶️ Usage

### 1. Display Help

```bash
./mp3reader --help
```

### 2. View MP3 Metadata

```bash
./mp3reader -v sample.mp3
```

### 3. Edit MP3 Metadata

**Update Title**

```bash
./mp3reader -e -t "New Song" sample.mp3
```

**Update Artist**

```bash
./mp3reader -e -a "New Artist" sample.mp3
```

**Update Album**

```bash
./mp3reader -e -A "New Album" sample.mp3
```

**Update Year**

```bash
./mp3reader -e -y "2026" sample.mp3
```

**Update Genre**

```bash
./mp3reader -e -m "Rock" sample.mp3
```

## 🎵 Supported ID3 Frames

| Command | Frame ID | Description |
| ------- | -------- | ----------- |
| `-t`    | TIT2     | Title       |
| `-a`    | TPE1     | Artist      |
| `-A`    | TALB     | Album       |
| `-y`    | TYER     | Year        |
| `-m`    | TCON     | Genre       |

## 🧠 C Concepts Implemented

- Command-Line Argument Handling (`argc`, `argv`)
- Structures and Pointers
- Dynamic Memory Allocation (`malloc`, `realloc`, `free`)
- File Operations (`fopen`, `fread`, `fwrite`, `fclose`)
- String Manipulation (`strcmp`, `strlen`, `strcpy`)
- Memory Operations (`memcpy`, `memcmp`)
- File Positioning (`fseek`)
- Binary File Processing
- Big-Endian to Little-Endian Conversion
- ID3 Synchsafe Integer Conversion

## 🔄 Working Process

1. Accept command-line arguments.
2. Validate the selected operation.
3. Open the original MP3 file in binary mode.
4. Read and validate the 10-byte ID3 header.
5. Extract the ID3 tag size.
6. Read metadata frames one by one.
7. Identify the requested frame.
8. Update the selected metadata.
9. Copy unchanged frames and audio data.
10. Write the updated header.
11. Replace the original MP3 file with the updated file.

## 📚 Learning Outcomes

Through this project, I gained practical experience in:

- Working with binary files.
- Understanding ID3v2.3 metadata structure.
- Handling dynamic memory allocation.
- Implementing file read and write operations.
- Working with pointers and structures.
- Understanding endian conversion.
- Developing a command-line application in C.

## ⚠️ Limitations

- Currently supports ID3v2.3.
- Comment (`COMM`) frame editing is not implemented.
- Text editing currently uses ISO-8859-1 encoding.
- Advanced ID3 features and frame-specific formats are not fully supported.

## 👨‍💻 Author

Developed as a C programming project to gain practical knowledge of file handling, pointers, dynamic memory allocation, and binary data processing.
