# SamLib Informer

An attempt to port famous android application [samlib-Info](https://github.com/monakhv/samlib-Info)
to other platforms. Now the project is at a very early stage. Nevertheless, now it can:
  * add an author to the DB;
  * remove the author (with all books and book groups) from the DB;
  * check updates for a specific author(s) or for all known authors;
  * mark author/group books/book as read;
  * mark a book as un-read;
  * show list of known authors/group books/books;
  * create directory structure for books;
  * downloads books in `FB2` or `HTML` format into file;

A simple command line interface is added. By default, it shows help:
```bash
Please say what should I do:

I know how to:
  --help                                Show this help messages
  -u [ --check-updates ]                Check for updates on all registered 
                                        authors
  --add arg                             Add new author
  --remove arg                          Remove author with given ID
  -l [ --list ] arg                     List [a[uthors]|g[roups]|b[ooks]]. For 
                                        books or groups you have to specify the
                                        `--author` option
  -m [ --mark-as ] arg                  Mark as [r[ead]|u[nread]] 
                                        -a|--author|-b|--book|-g|--group ID
  -s [ --show ]                         Show -a|--author|-b|--book|-g|--group 
                                        ID
  -a [ --author ] arg                   AuthorID
  -b [ --book ] arg                     BookID
  -g [ --group ] arg                    GroupID
  -n [ --new-only ]                     List only new/updated items
  --path-only                           Show only path to the local copy of the
                                        book with given BookID
  --location arg (="~/.local/share/SamLib/")
                                        Path to application data (e.g. DB, book
                                        storage etc)

```

Now you can easily add an author `SamlibInfo --add http://samlib.ru/s/sedrik/` or `SamlibInfo -a /s/saggaro_g/`. It will
automatically scan author's page and adds all books and book groups to the DB. 

To show list of know authors you may say e.g. `./SamlibInfo -l a` (or `./SamlibInfo --list author` ), for books say 
`./SamlibInfo -l b -a1` (or `./SamlibInfo --list=books --author=1`) etc.

Note, `*` next to the item's ID means there are updates. 

To see detail information about any book  #19 just say `./SamlibInfo -s -b19` (or `./SamlibInfo --show --book=19 `) and 
you'll get the next table
```
            ID: | 19
     Author ID: | 1
        Author: | Седрик
         Title: | Локи
         Genre: | Фэнтези, Фанфик  
   Description: | 
           URL: | http://samlib.ru/s/sedrik/loki.shtml
          Path: | file:///home/yourName/.local/share/SamLib/books/s/sedrik/loki.fb2.zip
  Size (delta): | 1376 (0)
   Has updates: | No
    Created at: | 2024-01-14 17:31:10
    Checked at: | 2024-01-14 17:31:10
```


## Conan
First of all, you need the [conan v2](https://docs.conan.io/2/installation.html) installed. You may not want to install
the `conan` "globally", as a system package/tool. So, just use [virtualenv](https://docs.python.org/3/library/venv.html)
```shell
python3 -m virtualenv venv
. ./venv/bin/activate
pip3 install conan
```

Before first run say `conan profile detect` to create a default [profile](https://docs.conan.io/2.0/reference/config_files/profiles.html).
Note, you will need a profile with `build_type=Debug`, while default profile is created with `build_type=Release`.

Then, if needed, install [ninja](https://github.com/ninja-build/ninja) or just say `pip3 install ninja`. 
After, export `core` and `cli` components:
```shell
conan export core
conan export cli
```

Note, you need to export them once you change something in the `CMakeLists.txt` or `conanfile.py` in those 
components/directories (e.g. `cli/CMakeLists.txt` or `cli/conanfile.py`).

Then install dependencies:

```shell
conan install . --build=missing --output=cmake-build-debug
```

Note you may need explicitly add the `-o *:shared=True` option to the `conan install` command. Without this `conan`
compiles dependencies as a **STATIC** libraries and therefore linker cannot link them to the project's binaries.

Now, you a ready to run `cmake`:

```shell
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_MAKE_PROGRAM=ninja \
      -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++ \
      -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake \
      -DCMAKE_TOOLCHAIN_FILE=cmake-build-debug/conan/conan_toolchain.cmake \
      -DCONAN_COMMAND=venv/bin/conan \
      -G Ninja \
      -S . \
      -B cmake-build-debug \
      --preset conan-debug
```

and build the project:
```shell
cmake --build ./cmake-build-debug --target SamlibInfo -j 10 --preset=conan-debug
```

### CLion users
Once you enable the `Conan` plugin, the `CLion` changes default `CMake` options for your project. In particular, it adds 
`CONAN_COMMAND`, `DCMAKE_PROJECT_TOP_LEVEL_INCLUDES` and `DCMAKE_TOOLCHAIN_FILE` parameters. 

You may want/need to adjust these parameters according you your paths:
  - `-DCONAN_COMMAND="~/SamlibInfo-cpp/venv/bin/conan"`
  - `-DCMAKE_TOOLCHAIN_FILE="~/SamlibInfo-cpp/cmake-build-debug/conan/conan_toolchain.cmake"`

Also, it might be helpful to add the `--preset conan-debug` option there. 

Note, these settings can be found here: 
  - `Settings` -> `Build, Execution, Deployment` -> `CMake` -> `Debug` -> `CMake options`
