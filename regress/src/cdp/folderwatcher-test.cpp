/*
   Bacula® - The Network Backup Solution

   Copyright (C) 2004-2019 Bacula Systems SA
   All rights reserved.

   The main author of Bacula is Kern Sibbald, with contributions from many
   others, a complete list can be found in the file AUTHORS.

   Licensees holding a valid Bacula Systems SA license may use this file
   and others of this release in accordance with the proprietary license
   agreement provided in the LICENSE file.  Redistribution of any part of
   this release is not permitted.

   Bacula® is a registered trademark of Kern Sibbald.
*/

#include "folderwatcher.h"
#include "btestutils.h"

class FolderWatcherTest: public FileChangeHandler
{
    char *_tmpDirPath;
    FolderWatcher *_watcher;
    alist _notifiedFiles;

    char *cpfile;
    char *cpfile2;

public:
    FolderWatcherTest(): _notifiedFiles(100, false) {
        setupTitle("FolderWatcherTest");

        _tmpDirPath = BTU::getTmpPath();
        logParam("Tmp Folder", _tmpDirPath);

        BTU::rmDir(_tmpDirPath);
        BTU::mkpath(_tmpDirPath, "");
        BTU::mkpath(_tmpDirPath, "test_rmdir1");
        BTU::mkpath(_tmpDirPath, "a1");
        BTU::mkpath(_tmpDirPath, "a1/b1");
        BTU::mkpath(_tmpDirPath, "a1/b1/c1");
        BTU::mkpath(_tmpDirPath, "a1/b1/c1/test_rmdir2");
        BTU::mkpath(_tmpDirPath, "a1/b1/c2");
        BTU::mkpath(_tmpDirPath, "a1/b2");
        BTU::mkpath(_tmpDirPath, "a1/b2/c1");
        BTU::mkpath(_tmpDirPath, "a1/b3");
        BTU::mkpath(_tmpDirPath, "a1/b3/c3");
        BTU::mkpath(_tmpDirPath, "a2");
        BTU::mkpath(_tmpDirPath, "a2/b3");
        BTU::mkpath(_tmpDirPath, "a2/b3/c3");

        char *mvfile = BTU::concat(_tmpDirPath, "/test_mv1.txt");
        char *delfile = BTU::concat(_tmpDirPath, "/test_del1.txt");
        char *delfile2 = BTU::concat(_tmpDirPath, "/a1/b3/test_del2.txt");
        char *touchfile = BTU::concat(_tmpDirPath, "/a1/b2/test_touch1.txt");
        cpfile = BTU::concat(_tmpDirPath, "/test_cp1.txt");
        cpfile2 = BTU::concat(_tmpDirPath, "/a1/b3/test_cp2.txt");

        BTU::mkfile(mvfile);
        BTU::mkfile(delfile);
        BTU::mkfile(delfile2);
        BTU::mkfile(touchfile);
        BTU::mkfile(cpfile);
        BTU::mkfile(cpfile2);

        _watcher = new FolderWatcher(this);
        char *err_msg = _watcher->watch(_tmpDirPath);
        
        if (err_msg != NULL) {
           printf("%s", err_msg);
           exit(-1);
        }
    }

    ~FolderWatcherTest() {
        //BTU::rmDir(_tmpDirPath);
    }

    void onChange(const char *fpath) {
        _notifiedFiles.append((void *) bstrdup(fpath));
    }

private:

    void verifyWatcherNotifiedChange(const char *fpath) {
        time_t start = clock();
        while (difftime(clock(), start) < 5000) {
            for (int i = 0; i < _notifiedFiles.size(); ++i) {
                char *file = (char *) _notifiedFiles.get(i);
                if (strcmp(file, fpath) == 0) {
                    printf("SUCCESS: notified %s\n\n", fpath);
                    return;
                }
            }

            usleep(1000); // 1 ms
        }

        printf("ERROR: watcher did not notify %s\n", fpath);
        exit(-1);
    }

    void verifyWatcherNotifiedNothing() {
        usleep(300000); // 300 ms

        if (_notifiedFiles.size() > 0) {
            for (int i = 0; i < _notifiedFiles.size(); ++i) {
                char *file = (char *) _notifiedFiles.get(i);
                printf("ERROR: notified %s\n", file);
            }
            exit(-1);
        }

        printf("SUCCESS: no notifications\n\n");
    }

public:

    void clear() {
        _notifiedFiles.destroy();
    }

    //TODO rename
    void watcher_should_notify_creation_of_new_file() {
        title("Watcher should notify creation of a new file");

        subtitle("1");
        char *file = BTU::concat(_tmpDirPath, "/test_file.txt");
        BTU::mkfile(file);
        verifyWatcherNotifiedChange(file);

        _notifiedFiles.destroy();

        subtitle("2");
        char *file2 = BTU::concat(_tmpDirPath, "/a1/b1/test_file2.txt");
        BTU::mkfile(file2);
        verifyWatcherNotifiedChange(file2);

        _notifiedFiles.destroy();

        subtitle("3");
        char *file3 = BTU::concat(_tmpDirPath, "/a2/test_file3.txt");
        BTU::mkfile(file3);
        verifyWatcherNotifiedChange(file3);

        _notifiedFiles.destroy();

        subtitle("4");
        BTU::mkpath(_tmpDirPath, "a2/b3/new_folder");
        verifyWatcherNotifiedNothing();

        char *file4 = BTU::concat(_tmpDirPath, "/a2/b3/new_folder/test_file4.txt");
        BTU::mkfile(file4);
        verifyWatcherNotifiedChange(file4);

        _notifiedFiles.destroy();

        subtitle("5");
        char *targetFile = BTU::concat(_tmpDirPath, "/a1/b2/copied1.txt");
        BTU::cpFile(cpfile, targetFile);
        verifyWatcherNotifiedChange(targetFile);

        _notifiedFiles.destroy();

        subtitle("6");
        targetFile = BTU::concat(_tmpDirPath, "/a2/b3/c3/copied2.txt");
        BTU::cpFile(cpfile2, targetFile);
        verifyWatcherNotifiedChange(targetFile);
    }

    void watcher_should_notify_nothing_on_file_removal() {
        title("Watcher should notify nothing on file removal");

        subtitle("1");
        char *file = BTU::concat(_tmpDirPath, "/test_del1.txt");
        BTU::rmFile(file);
        verifyWatcherNotifiedNothing();

        _notifiedFiles.destroy();

        subtitle("2");
        char *file2 = BTU::concat(_tmpDirPath, "/a1/b3/test_del2.txt");
        BTU::rmFile(file2);
        verifyWatcherNotifiedNothing();
    }

    void watcher_should_notify_changes_on_file() {
        title("Watcher should notify changes on a file");

#ifndef HAVE_WIN32
        subtitle("1");
        char *file1 = BTU::concat(_tmpDirPath, "/a1/b2/test_touch1.txt");
        BTU::touch(file1);
        verifyWatcherNotifiedChange(file1);

        _notifiedFiles.destroy();

        subtitle("2");
        char *file2 = BTU::concat(_tmpDirPath, "/test_mv1.txt");
        char *file3 = BTU::concat(_tmpDirPath, "/a1/b1/c1/test_mv1_moved.txt");
        BTU::mvFile(file2, file3);
        verifyWatcherNotifiedChange(file3);
#endif
    }

    void watcher_should_notify_nothing_on_creation_of_new_dir() {
        title("Watcher should notify nothing on creation of a new directory");

        subtitle("1");
        BTU::mkpath(_tmpDirPath, "a42");
        verifyWatcherNotifiedNothing();

        _notifiedFiles.destroy();

        subtitle("2");
        BTU::mkpath(_tmpDirPath, "a100");
        verifyWatcherNotifiedNothing();

        _notifiedFiles.destroy();

        BTU::mkpath(_tmpDirPath, "a100/b57");
        verifyWatcherNotifiedNothing();
    }

    void watcher_should_notify_nothing_on_dir_removal() {
        title("Watcher should notify nothing on directory removal");

        subtitle("1");
        char *dir1 = BTU::concat(_tmpDirPath, "/test_rmdir1");
        BTU::rmDir(dir1);
        verifyWatcherNotifiedNothing();

        _notifiedFiles.destroy();

        subtitle("2");
        char *dir2 = BTU::concat(_tmpDirPath, "/a1/b1/c1/test_rmdir2");
        BTU::rmDir(dir2);
        verifyWatcherNotifiedNothing();
    }

    void watcher_should_not_notify_changes_on_dir_metadata() {
        title("Watcher should notify nothing on directory metadata changes");

        subtitle("1");
        char *dir = BTU::concat(_tmpDirPath, "/a1");
        BTU::touch(dir);
        verifyWatcherNotifiedNothing();
    }

    void delete_watcher_should_stop_watch() {
        title("Deleting watcher should stop watch");
        delete _watcher;
    }
};

int main(int arg, char *argv[])
{
    lmgr_init_thread();
    FolderWatcherTest test;
    test.watcher_should_notify_creation_of_new_file();
    test.clear();
    test.watcher_should_notify_nothing_on_file_removal();
    test.clear();
    test.watcher_should_notify_changes_on_file();
    test.clear();
    test.watcher_should_notify_nothing_on_creation_of_new_dir();
    test.clear();
    test.watcher_should_notify_nothing_on_dir_removal();
    test.clear();
    test.watcher_should_not_notify_changes_on_dir_metadata();
    test.clear();
}
