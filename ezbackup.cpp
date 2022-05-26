#include "ezbackup.h"
#include "ui_ezbackup.h"
#include "ppl.h"

EZBackup::EZBackup(QWidget *parent) :
    QMainWindow(parent),  taskbarProgress(0),  taskbarButton(0),
    ui(new Ui::EZBackup)
{
    ui->setupUi(this);

    //if the user manually selected the core compressor or not, by default no
    Sel_7zip=false;

    // initial address in the mega
    mega_dir_surf<<"Root";

    //fill the address lable of the program with the address of 7zip plugin
    program=QCoreApplication::applicationDirPath()+"\\7z.exe";
    ui->LBL_SEL_7ZIP->setText(program);

    interupt=false;

    //local=false   cloud=true
    local_or_cloud=false;

    // Disable the maximize button of windows
    this->setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);

    // Password boxes shod be set up to shot *
    ui->TXT_PASS_1->setEchoMode(QLineEdit::Password);
    ui->TXT_PASS_1->setInputMethodHints(Qt::ImhHiddenText| Qt::ImhNoPredictiveText|Qt::ImhNoAutoUppercase);

    ui->TXT_PASS_2->setEchoMode(QLineEdit::Password);
    ui->TXT_PASS_2->setInputMethodHints(Qt::ImhHiddenText| Qt::ImhNoPredictiveText|Qt::ImhNoAutoUppercase);


    connect(ui->BTN_SRC, SIGNAL(clicked()), this, SLOT(Read_src_dir_btn_click()));
    connect(ui->BTN_DST, SIGNAL(clicked()), this, SLOT(Read_dst_dir_btn_click()));
    connect(ui->BTN_BACKUP, SIGNAL(clicked()), this, SLOT(btn_backup_click()));
    connect(ui->BTN_SEL_7ZIP, SIGNAL(clicked()), this, SLOT(btn_sel_7zip_click()));
    connect(ui->BTN_CANCEL, SIGNAL(clicked()), this, SLOT(btn_cancel_click()));
    connect(ui->BTN_FILE_TYPE_ADD, SIGNAL(clicked()), this, SLOT(btn_file_type_add_click()));
    connect(ui->BTN_SAVE_REPORT, SIGNAL(clicked()), this, SLOT(btn_save_report_click()));

    taskbarButton = new QWinTaskbarButton(parent);
    //taskbarButton->setParent(parent);
    taskbarButton->setWindow(windowHandle());

    taskbarProgress = taskbarButton->progress();
    connect(ui->PGRS_BAR, SIGNAL(valueChanged(int)), taskbarProgress, SLOT(setValue(int)));
    connect(ui->PGRS_BAR, SIGNAL(valueChanged(int)), taskbarProgress, SLOT(show()));



    //This part is to sent keyboard shortcuts for the file type Listbox and Textbox
    QShortcut* shortcut_del = new QShortcut(QKeySequence(Qt::Key_Delete), ui->LST_FILE_TYPE);
    connect(shortcut_del, SIGNAL(activated()), this, SLOT(deleteItem()));

    QShortcut* shortcut_enter1 = new QShortcut(QKeySequence(Qt::Key_Return), ui->TXT_FILE_TYPE);
    connect(shortcut_enter1, SIGNAL(activated()), this, SLOT(btn_file_type_add_click()));

    QShortcut* shortcut_enter2 = new QShortcut(QKeySequence(Qt::Key_Enter), ui->TXT_FILE_TYPE);
    connect(shortcut_enter2, SIGNAL(activated()), this, SLOT(btn_file_type_add_click()));

    // Setup th image for Abput Tab, read it from Resources
    QPixmap img_about(":/images/about_image.png");
    ui->IMG_ABOUT->setPixmap(img_about);
    ui->IMG_ABOUT->setScaledContents(1);

    // Add required initial values to the drop down menus
    ui->LST_FILE_TYPE->addItem("*.jpg");

    ui->CMB_ZIP_TYPE->addItem(".7z");
    ui->CMB_ZIP_TYPE->addItem(".zip");

    ui->CMB_COMPRESS->addItem("mx0");
    for (int i=1; i<10; i=i+2)
        ui->CMB_COMPRESS->addItem("mx"+QString::number(i));

    //In the begining, the cancel button shoud be invisible
    ui->BTN_CANCEL->setVisible(false);



    taskbarProgress->setValue(0);


}

EZBackup::~EZBackup()
{
    delete ui;
}


void EZBackup::print(QString text, bool line, QColor color)
{
    if (!line)
        delete(ui->LST_STAT->item(ui->LST_STAT->count()-1));

    if (ui->CHK_LST_CLR->isChecked())
        if (ui->LST_STAT->count()>100)
            ui->LST_STAT->clear();

    ui->LST_STAT->addItem(text);
    ui->LST_STAT->item(ui->LST_STAT->count()-1)->setBackground(color);
    ui->LST_STAT->scrollToBottom();
    ui->LST_STAT->update();
}


void EZBackup::Read_src_dir_btn_click()
{
    ui->LST_STAT->clear();
    src_addr = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"C:\\", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (ui->LBL_SRC->text().length()>0)
    {
        ui->LBL_SRC->setText(src_addr);
        print("Source: "+src_addr, true ,Qt::color0);
    }
    else
    {
        print("Invalid source directory is selected!", true ,Qt::red);
    }
}

void EZBackup::Read_dst_dir_btn_click()
{
    dst_addr = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"C:\\", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (ui->LBL_DST->text().length()>0)
    {
        ui->LBL_DST->setText(dst_addr);
        print("Destination: "+dst_addr, true ,Qt::color0);
    }
    else
    {
        print("Invalid destination directory is selected!", true ,Qt::red);
    }
}


void EZBackup::btn_backup_click()
{

    // it is required to hide the back up bitton and show the cancel button
    ui->BTN_BACKUP->setVisible(false);
    ui->BTN_CANCEL->setVisible(true);

    // passing arrey, error code, n of new zips, n of existed zips, n of new folders, n of existed folders
    for (int i=0;i<5;i++)
        info[i]=0;

    //check to see if any directory has been selected, else report error no 3
    if ((src_addr.length()==0)||(dst_addr.length()==0))
        info[0]=3;

    ui->LBL_SEL_7ZIP->setText(program);
    if (Sel_7zip)
        program = ui->LBL_SEL_7ZIP->text();

    // show status tab
    ui->TAB->setCurrentIndex(1);

    // if the list of file type are empty, it should not zip any thing
    if (ui->LST_FILE_TYPE->count()==0)
        info[0]=1;

    //call directory_check and check all directories and create them if necessary
    directory_check(info);

    //creating dynamic arrey for text report
    new_files=new QStringList[info[3]+info[4]+1];


    // This part, parses the file type list and append filetypes into filters
    QStringList filters;
    for (int i=0; i<ui->LST_FILE_TYPE->count();i++)
        filters<<ui->LST_FILE_TYPE->item(i)->text();

    int i=0;
    while((i<all_src_dirs.count())&&(info[0]==0)&&(!interupt))
    {
        QDir check_dir(all_src_dirs.at(i));

        //filters << "*.jpg";
        check_dir.setNameFilters(filters);

        // do the filter on file types and put the results in list
        QStringList list=check_dir.entryList(filters);

        int j=0;
        while((j<list.count())&&(info[0]==0)&&(!interupt))
        {
            QFileInfo checkFile(all_dst_dirs.at(i)+"//"+list.at(j)+".7z");

            // check if file exists and if yes: Is it really a file and no directory?
            if (checkFile.exists() && checkFile.isFile())
            {
                print("File exists: " + all_dst_dirs.at(i)+"/"+list.at(j)+".7z",true, Qt::darkGray);
                info[2]++;
            }
            else
            {
                info[0]=encode(all_src_dirs.at(i), all_dst_dirs.at(i), list.at(j));
                if (!info[0]){
                    print("Backing up: " + all_dst_dirs.at(i)+"/"+list.at(j)+".7z",true, Qt::darkGreen);
                    info[1]++;
                    new_files[i].push_back(list.at(j)+".7z");
                 }

            }
            delay(1);
            ui->PGRS_BAR->setValue(((i+1)+((j+1)/list.count()))*100/all_src_dirs.count());
            //this->taskbar->SetProgressValue((HWND)winId(), ((i+1)+((j+1)/list.count()))*100/all_src_dirs.count(), 100);
//            taskbar_progress->setValue(((i+1)+((j+1)/list.count()))*100/all_src_dirs.count());
//            taskbar_progress->show();
//            taskbar_progress->resume();
//            ui->PGRS_BAR->update();

            j++;
        }

        i++;
        list.clear();
    }

    ui->PGRS_BAR->setValue(100);
    ui->PGRS_BAR->update();

    //print(QString::number(taskbarProgress->), true, Qt::red);

    //info[0]=error;
    error_management(info);

    // now it is time to hide the cancel button and show bachup button again
    ui->BTN_CANCEL->setVisible(false);
    ui->BTN_BACKUP->setVisible(true);

    interupt=false;

}

void EZBackup::directory_check(int *info)
{
    // Now prepare source and destination directories to be labled and compared
    QDirIterator directories_src(src_addr, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    QDirIterator directories_dst(dst_addr, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    if (ui->CHK_IMG_BCKUP->checkState())
    {
        QDirIterator directories_dst_img_bckup(dst_addr_img_bckup, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        all_dst_img_bckup_dirs<<directories_dst_img_bckup.path();
    }
    // the first entry of list is the root of addresses
    all_src_dirs <<directories_src.path();
    all_dst_dirs <<directories_dst.path();



    // create same directories as source in the destination
    while(directories_src.hasNext()&&(!interupt)&&(!info[0])){
        directories_src.next();
        all_src_dirs << directories_src.filePath();

        //constructing the path of zip destination
        QString check_path=dst_addr+all_src_dirs.last().right(all_src_dirs.last().count()-all_src_dirs.at(0).count());

        //copy it to the list
        all_dst_dirs << check_path;

        //check if the folder exists or not
        QDir dir(check_path);

        if (!dir.exists())
        {
            QDir().mkpath(check_path);
            print("Creating directory: "+check_path,true, Qt::darkGreen);
            info[3]++;
        }
        else
        {
            print("Directory exists: "+check_path,true, Qt::darkGray);
            info[4]++;
        }
        delay(5);

        // Now do the same for the image backup stage if the checkbox is marked
        if (ui->CHK_IMG_BCKUP->checkState())
        {
            //constructing the path of image destination
            QString check_path_img_bckup=dst_addr_img_bckup+all_src_dirs.last().right(all_src_dirs.last().count()-all_src_dirs.at(0).count());

            //copy it to the list
            all_dst_img_bckup_dirs << check_path_img_bckup;

            //check if the folder exists or not
            QDir dir_2(check_path_img_bckup);

            if (!dir_2.exists())
            {
                QDir().mkpath(check_path_img_bckup);
                print("Creating directory: "+check_path_img_bckup,true, Qt::darkGreen);
                info[3]++;
            }
            else
            {
                print("Directory exists: "+check_path_img_bckup,true, Qt::darkGray);
                info[4]++;
            }
            delay(5);
        }

    }

}

void EZBackup::btn_sel_7zip_click()
{
    QString file1Name = QFileDialog::getOpenFileName(this, tr("Open 7z.exe File 1"), "c:\\program files", tr("7z.exe"));
    ui->LBL_SEL_7ZIP->setText(file1Name);
    Sel_7zip=true;
}

int EZBackup::encode(QString src, QString dst, QString filename)
{
    QStringList arguments;

    if ((ui->CHK_PASS->isChecked())&& (ui->TXT_PASS_1->text().length()>0))
    {
        if (ui->TXT_PASS_1->text()==ui->TXT_PASS_2->text())
        {
            arguments <<"a"<<"-"+ui->CMB_COMPRESS->currentText()<<"-mmt"<<"-t7z"<<"-p"+ui->TXT_PASS_1->text()<<dst+"/"+filename+ui->CMB_ZIP_TYPE->currentText()<<src+"/"+filename;
            QProcess *myProcess = new QProcess();
            myProcess->start(program,arguments);
            myProcess->waitForFinished(-1);
            delete myProcess;
            delay(10);
            return 0;
        }
        else
        {
            return 2;
        }
    }
    else if (!ui->CHK_PASS->isChecked())
    {
        arguments <<"a"<<"-"+ui->CMB_COMPRESS->currentText()<<"-mmt"<<"-t7z"<<dst+"/"+filename+ui->CMB_ZIP_TYPE->currentText()<<src+"/"+filename;
        QProcess *myProcess = new QProcess();
        myProcess->start(program,arguments);
        myProcess->waitForFinished(-1);
        delete myProcess;
        delay(10);
        return 0;
    }
    else
    {
        return 2;
    }

}


void EZBackup::error_management(int *info)
{

    switch (info[0])
    {
    case 3:
        print("Source of destination is not valid",true, Qt::color0);
        QMessageBox::critical(this, tr("EZBackup"),tr("Verify source or destination addresses!"));
        break;
    case 2:
        ui->TXT_PASS_1->clear();
        ui->TXT_PASS_2->clear();
        print("Password error",true, Qt::color0);
        QMessageBox::warning(this, tr("EZBackup"),tr("Please enter the password carefully!"));
        break;
    case 1:
        print("No file type is selected to back up!",true, Qt::color0);
        QMessageBox::critical(this, tr("EZBackup"),tr("Please choose file types to back up!"));
        break;

    case 0:
        if (interupt)
        {
            print("Operation canceled by user",true, Qt::color0);
            print("Report:",true, Qt::color0);
            print("      Directories created: " + QString::number(info[3]),true, Qt::color0);
            print("      Files backed up: "+QString::number(info[1]),true, Qt::color0);
            print("      Directories existed before: "+QString::number(info[4]),true, Qt::color0);
            print("      File existed brfore: "+QString::number(info[2]),true, Qt::color0);
            QMessageBox::information(this, tr("EZBackup"),tr("Operation canceled by user!"));
        }
        else
        {
            print("Finished Successfully",true, Qt::color0);
            print("Report:",true, Qt::color0);
            print("      Directories created: " + QString::number(info[3]),true, Qt::color0);
            print("      Files backed up: "+QString::number(info[1]),true, Qt::color0);
            print("      Directories existed before: "+QString::number(info[4]),true, Qt::color0);
            print("      File existed brfore: "+QString::number(info[2]),true, Qt::color0);
            QMessageBox::information(this, tr("EZBackup"),tr("Finished Successfully"));
        }

        break;
    default:
        QMessageBox::information(this, tr("EZBackup"),tr("Unknown Error"));
        print("Unknown Error is reported",true, Qt::color0);
        break;
    }
}

void EZBackup::delay(int t)
{
    QTime dieTime= QTime::currentTime().addMSecs(t);
    while( QTime::currentTime() < dieTime )
    QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
}

void EZBackup::btn_cancel_click()
{
    interupt=true;

    ui->BTN_CANCEL->setVisible(false);
    ui->BTN_BACKUP->setVisible(true);
}

void EZBackup::btn_file_type_add_click()
{
    ui->LST_FILE_TYPE->addItem(ui->TXT_FILE_TYPE->text());
    ui->TXT_FILE_TYPE->clear();
    ui->TXT_FILE_TYPE->setText("*.");
}

void EZBackup::deleteItem()
{
    delete ui->LST_FILE_TYPE->currentItem();
}

void EZBackup::btn_save_report_click()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save file", "Report.txt", "*.txt");

    QFile f(filename);

    f.open( QIODevice::WriteOnly );

    QTextStream out( &f );
    // store data in f

    for (int i=0;i<all_src_dirs.count();i++)
    {
        out<<"Directory: "<<all_src_dirs.at(i)<<endl;

        for (int j=0; j<new_files[i].count();j++)
        {
            if (new_files[i].length()>0)
            {
                out<<"   Added file: "<<new_files[i].at(j)<<endl;
            }

        }
        if (new_files[i].count()==0)
            out<<"   None"<<endl;

        out<<endl;
    }

    out<<"-------------------------------------"<<endl;
    out<<"      Directories created: "<<QString::number(info[3])<<endl;
    out<<"      Files backed up: "<<QString::number(info[1])<<endl;
    out<<"      Directories existed before: "<<QString::number(info[4])<<endl;
    out<<"      File existed brfore: "<<QString::number(info[2])<<endl;

    f.close();
}


