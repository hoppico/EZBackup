#ifndef EZBACKUP_H
#define EZBACKUP_H

#include <QMainWindow>

#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QStandardItemModel>
#include <QDirIterator>
#include <qt_windows.h>
#include <QProcess>
#include <QMessageBox>
#include <QShortcut>
#include <windows.h>


#include <QtWinExtras>

#include <QWidget>

#include <QTime>
#include <QColor>

namespace Ui {
class EZBackup;
}

class EZBackup : public QMainWindow
{
    Q_OBJECT

public:
    // necessary addresses
    QString src_addr;
    QString dst_addr;
    QString dst_addr_img_bckup;

    // show the results in the output status
    void print(QString text, bool line, QColor color);

    // performs tha zip alg
    int encode(QString src, QString dst, QString filename);

    // manage errors to show proper message to user
    void error_management(int *info);

    // add delay to curb system freezing
    void delay(int t);
    void directory_check(int *info);
    void cloud_zip_upload();
    void cloud_directory_check(int *info);

    // to create small images from is the image backup is selected
    void img_backup();

    explicit EZBackup(QWidget *parent = 0);
    ~EZBackup();

private:
    Ui::EZBackup *ui;

    bool Sel_7zip;
    QString program;
    bool interupt;
    bool local_or_cloud;
    // passing arrey, error code, n of new zips, n of existed zips, n of new folders, n of existed folders
    int info[5];

    // All directories in the source and destination
    QStringList all_src_dirs, all_dst_dirs, all_dst_img_bckup_dirs;

    QStringList *new_files;

    QStringList mega_dir_surf;
    QString mega_dir_loc;

#ifdef Q_OS_WIN
    QWinTaskbarButton* taskbarButton;
    QWinTaskbarProgress* taskbarProgress;
#endif

public slots:
    void Read_src_dir_btn_click();
    void Read_dst_dir_btn_click();
    void btn_backup_click();
    void btn_sel_7zip_click();
    void btn_cancel_click();
    void btn_file_type_add_click();
    void deleteItem();
    void btn_save_report_click();
};

#endif // EZBACKUP_H
