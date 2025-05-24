# Fat32 Action Tuner

> A simple fat32 image file interactor. For the Operating System Course of SCSE, BUAA.
>
> Pay attention: No comprehensive testing has been conducted and no comprehensive error handling and rollback mechanisms have been implemented. **Please backup the image file MANULLY**!

## Compile

Just compile with no addition configs:

```
make -j`nproc`
```

Or compile with debug information:

```
CFLAGS=-g make -j`nproc`
```

Or compile with optimization:

```
CFLAGS=-O3 make -j`nproc`
```

You can run the following targets to clean the repo:

```
make tidy  # remove all the obj files
make clean # remove all the files compiled, include the executable binary
```

## Run Args

- `-q`: Quiet mode, will shutdown all the logs.
- `-v`: Verbose mode 1, will print more logs.
- `-vv`: Verbose mode 2, will print all logs.
- `-f host/path/to/fat32.img`: Load the image file directly.
- `-e "command1" -e "command2" ...`: Run the commands one by one and then exit the program.

It is adviced to put the `-q` first if needed.

## Commands in CLI

The CLI will seperate all arguments just according to the space. So you cannot include any spaces inside the arguments.

The available commands and their descriptions are as follows:

- `help`: No Args. Show the built-in help message.
- `dummy`: Any Args. Print the arguments one by one. For test.
- `load <host/path/to/fat32.img>`: Need One Arg. Load the fat32 image file in host disk. Will unload the loaded image if exists.
- `unload`: No Args. Unload the loaded image if exists.
- `info`: No Args. Show some informations of the loaded fat32 image.
- `ls [part1 [part2 [...]]]`: Any Args. Retrive the items in path /part1/part2/... . If it is a directory, show the items inside. If it is a file, print the contents. Root directory will be used if no args provided.
- `touch [part1 [part2 [...]]] filename`: At Least One Args. Create a new file named 'filename' in path /part1/part2/... .
- `mkdir [part1 [part2 [...]]] dirname`: At Least One Args. Create a new directory named 'dirname' in path /part1/part2/... .
- `rm [part1 [part2 [...]]]`: Any Args. Remove the file or empty directory with path /part1/part2/... . You cannot remove the root directory.
- `mv [part11 [part12 [...]]] -> [part21 [part22 [...]]] filename`: At Least Three Args. Move the file or directory with path /part11/part12/... into /part21/part22/... . Full path including the name of destination is needed.
- `truncate number [part1 [part2 [...]]]`: At Least One Args. Modify the size of file with path /part11/part12/... into 'number' bytes. Cluster will be released or allocated. If the 'number' is '0', the file will NOT be removed. The 'number' can be greater than the current size.
- `exit [code]` OR `quit [code]`: No Or One Args. Quit the program. If the 'code' is present, set the return value with it.
- `.fat.read cluster`: One Arg. Print the contents in cluster chain start from 'cluster'.
- `.fat.ls cluster`: One Arg. Treat the contents in cluster chain start from 'cluster' as directory entries. Print the items information in it.

## Examples

1. Create a directory `/folder`:

```
>>> mkdir folder
```

2. Create a file in `/folder`:

```
>>> touch folder file.txt
```

3. Allocate 1000 bytes for `/folder/file.txt`:

```
>>> truncate 1000 folder file.txt
```

4. Move the `/folder/file.txt` into the Root Directory:

```
>>> mv folder file.txt -> file.txt
```

5. Retrive all items in the Root Directory:

```
>>> ls
```

6. Remove the `/folder`:

```
>>> rm folder
```
