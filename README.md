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