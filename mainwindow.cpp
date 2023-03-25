#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

#define L(name) ui->lbl_##name##_a
  for (auto l : { L(ad), L(family), L(name), L(setup), L(tel) })
    l->setVisible(false);
#undef L


  // init db
  InitDB();
  InitModel(ui->tableView_3);
  ui->tableView_3->showGrid();

  // init connections and timer
  timer = new QTimer(this);
  connect(ui->btn_search, SIGNAL(clicked(bool)), this, SLOT(search()));
  connect(ui->btn_add, SIGNAL(clicked(bool)), this, SLOT(add()));
  connect(timer, SIGNAL(timeout()), this, SLOT(alarm()));

  connect(ui->t_setupday, SIGNAL(textChanged()), this,
          SLOT(on_TextChange_setupday()));
  connect(ui->t_setupmounth, SIGNAL(textChanged()), this,
          SLOT(on_TextChange_setupmounth()));
  connect(ui->t_setupyear, SIGNAL(textChanged()), this,
          SLOT(on_TextChange_setupyear()));

  connect(ui->btn_revival6,&QPushButton::clicked,this,&MainWindow::six_mounth_update);
  connect(ui->btn_revival1,&QPushButton::clicked,this,&MainWindow::one_year_update);
  connect(ui->btn_revival2,&QPushButton::clicked,this,&MainWindow::two_year_update);

  QDate date;
  date = QDate::currentDate();
  if(!date.isValid())
  {
      QMessageBox bx;
      bx.setText("لطفا تاریخ سیستم را به میلادی تغییر دهید "
                 "برنامه به درستی کار نخواهد کرد");
      bx.exec();
  }
  int ey, em, ed;
  ey = em = ed = 0;
  gregorian_to_jalali(&ey, &em, &ed, date.year(), date.month(),
                      date.day());
  ui->lbl_date->setText(QSTI(ey) + "/" + QSTI(em) + "/" + QSTI(ed));
  QTime time=QTime::currentTime();
  ui->lbl_time->setText(time.toString("h:m"));
}

MainWindow::~MainWindow()
{
  delete ui;
  Close_DB();
}
void
MainWindow::InitDB()
{
  //   QDateConvertor mdate;
  //   QStringList today= mdate.Today();
  //   qDebug()<<today.at(0)<<"/"<<today.at(1)<<"/"<<today.at(2)<<"/"<<today.at(3)<<"/"<<today.at(4);

  QString path = QCoreApplication::applicationDirPath() + "/dbh.dbo";
  db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName(path);

  new_db = CreateDbIfNotExist(path); // if new database create then true
  bool ok = db.open();
  if (!ok) {
    qDebug() << db.lastError().text();
    qDebug() << __func__ << __FILE__ << __LINE__;
    exit(0);
  }
  query = new QSqlQuery(db);
  model = new MyModel(this);

  if (new_db)
    CreateTables(); // create tables
}
void
MainWindow::InitModel(QTableView* _view)
{
  view = _view;
  QString query1 = "SELECT "
                   "person.id,"
                   "person.name,"
                   "person.family,"
                   "person.address,"
                   "person.tel,"
                   "person.devname,"
                   "person.serial,"
                   "person.setup "
                   "FROM person ";

  model->setQuery(query1, db);

  {
      int i = 0;
      for (auto o :
      { QObject::tr("رديف"), QObject::tr("نام"), QObject::tr("نام خانوادگي"),
           QObject::tr("آدرس"), QObject::tr("تلفن"), QObject::tr("نام دستگاه"),
           QObject::tr("سريال"),QObject::tr("تاریخ") }  ) {
          model->setHeaderData(i++, Qt::Horizontal, o);
      }
  }
  view->setModel(model);
  view->show();
}
bool
MainWindow::CreateDbIfNotExist(const QString& path_)
{
  if (!QFile::exists(path_)) {
    QFile file(path_);
    if (file.open(QFile::OpenModeFlag::ReadWrite)) {
      // file created successfully
      file.close();
    } else {
      // error can not open file
      qDebug() << "Can not open file : " << path_;
      return false;
    }
    return true;

  } else
    return false;
}
void
MainWindow::Close_DB()
{
  db.close();
  delete query;
}
#define CHECK_QURY_ERROR(qe)                                                   \
  MainWindow::CHECK_QURY_ERROR_F(qe, __func__, __FILE__, __LINE__)
void inline MainWindow::CHECK_QURY_ERROR_F(const bool& query_exec,
                                           const char* func, const char* file,
                                           int line)
{
  if (!query_exec) {
    qDebug() << query->lastError().text();
    qDebug() << func << file << line;
    exit(0);
  }
}
void inline MainWindow::CreateTables()
{
    QString person_query = "CREATE TABLE person( "
                         "id INTEGER primary key autoincrement,"
                         "name string,"
                         "family string,"
                         "address string,"
                         "tel string,"
                         "devname string,"
                         "serial string,"
                         "setup string,"
                         "change_six string,"
                         "change_one string,"
                         "change_two string);";

  CHECK_QURY_ERROR(query->exec(person_query));
  // CHECK_QURY_ERROR(query->exec("PRAGMA foreign_keys = ON;"));
}
void
MainWindow::search_query(const QString& pid, const QString& name,
                         const QString& family, const QString& address,
                         const QString& tel, const QString& devname,
                         const QString& serial,const QString& setup)
{
  /******* mrn *********/

  QString _pid = "1";
  QString _name = "1";
  QString _family = "1";
  QString _address = "1";
  QString _tel = "1";
  QString _devname = "1";
  QString _serial = "1";
//  QString _filtertype = "1";
  QString _setup = "1";

  if (pid != "")
    _pid = "(person.id='" + pid + "')";
  if (name != "")
    _name = "(person.name LIKE '" + name + "%')";
  if (family != "")
    _family = "(person.family LIKE '" + family + "%')";
  if (address != "")
    _address = "(person.address LIKE '" + address + "%')";
  if (tel != "")
    _tel = "(person.tel='" + tel + "')";
  if (devname != "")
    _devname = "(person.devname='" + devname + "')";
  if (serial != "")
    _serial = "(person.serial='" + serial + "')";
  if (setup != "")
    _setup = "(person.setup='" + setup + "')";

  QString s_query = "SELECT "
                    "person.id,"
                    "person.name,"
                    "person.family,"
                    "person.address,"
                    "person.tel,"
                    "person.devname,"
                    "person.serial "
                    "FROM person "
                    "WHERE " +
                    _pid + " AND " + _name + " AND " + _family + " AND " +
                    _address + " AND " + _tel + " AND " + _devname +  " ;";
  qDebug() << s_query;

  if(query->exec(s_query))
  {
     // ui->lbl_status->setText(QString("وضعیت : جست و جو انجام شد"));

  }else
  {
     //ui->lbl_status->setText(QString("وضعیت : جست و جو با مشکل مواجه شد"));
     CHECK_QURY_ERROR(false);;
  }


  model->setQuery(*query);
  view->setModel(model);
}

void
MainWindow::add_query(const QString& name, const QString& family,
                      const QString& address, const QString& tel,
                      const QString& devname, const QString& serial,
                      const QString& setup)
{
    bool retval;
  // query->prepare("select person.id from person where name=name")
  query->prepare("INSERT INTO person \
          (name,family,address,tel,devname,serial,setup,change_six,change_one,change_two ) \
    values(?,       ?,    ?,    ?,  ?,        ?,    ?,     ?,          ?,      ?        );");


  query->addBindValue(name);
  query->addBindValue(family);
  query->addBindValue(address);
  query->addBindValue(tel);
  query->addBindValue(devname);
  query->addBindValue(serial);
  query->addBindValue(setup);

  ExpireData exp_date; // yy-mm-dd
  //automatic calculating expire date
  //1- expire after six mounth
  //2- expire after a year
  //3- expire after two year
  exp_date = expire_date(setup);

  query->addBindValue(exp_date.six_mounth);
  query->addBindValue(exp_date.one_year);
  query->addBindValue(exp_date.two_year);
  retval = query->exec();
  CHECK_QURY_ERROR(retval);
  ui->t_id->setText("0");

  if(retval)
  {
      ui->lbl_status->setText(QString("وضعیت : دخیره اطلاعات انجام شد"));

  }else
  {
    ui->lbl_status->setText(QString("وضعیت : دخیره نشد"));
  }
  // model->setQuery(*query);
  // view->setModel(model);
  // search();
}
bool
MainWindow::check_fields()
{
  volatile bool res = true;
  timer_counter = 0;
  timer->setInterval(500);
  QMessageBox msg;
  if (ui->t_setupday->toPlainText().length() > 2 && !valid_day_check) {
    msg.setText(QObject::tr("لطفا روز را تصحیح کنید."));
    ui->lbl_status->setText(QString("وضعیت : لطفا روز را تصحیح کنید"));
    msg.exec();
    res = false;
  } else if (ui->t_setupmounth->toPlainText().length() > 2 &&
             !valid_mount_check) {
    msg.setText(QObject::tr("لطفا ماه را تصحیح کنید."));
    ui->lbl_status->setText(QString("وضعیت : لطفا ماه را تصحیح کنید"));
    msg.exec();
    res = false;
  } else if (ui->t_setupyear->toPlainText().length()!=4 && !valid_year_check) {
    msg.setText(QObject::tr("لطفا سال را تصحیح کنید."));
    ui->lbl_status->setText(QString("وضعیت : لطفا سال را تصحیح کنید"));
    msg.exec();
    res = false;
  }
  if (ui->t_name->toPlainText() == "" || ui->t_family->toPlainText() == "" ||
      ui->t_address->toPlainText() == "" || ui->t_tel->toPlainText() == "" ||
      ui->t_setupday->toPlainText() == "" ||
      ui->t_setupmounth->toPlainText() == "" ||
      ui->t_setupyear->toPlainText() == "") {
    timer->start();
    res = false;
  }
  return res;
}
void
MainWindow::set_feilds(const QString& id, const QString& name,
                       const QString& family, const QString& address,
                       const QString& tel, const QString& devicename,
                       const QString& deviceserial,
                       const QString& setup, const ExpireData exp)
{
  ui->t_id->setText(id);
  ui->t_name->setText(name);
  ui->t_family->setText(family);
  ui->t_address->setText(address);
  ui->t_tel->setText(tel);
  ui->t_devicename->setText(devicename);
  ui->t_deviceserial->setText(deviceserial);

  // date to yy-mm-dd
  if (setup == "") {
    ui->t_setupyear->setText("");
    ui->t_setupmounth->setText("");
    ui->t_setupday->setText("");
  } else {
    QStringList lst1 = setup.split("-");
    ui->t_setupyear->setText(lst1.at(0));
    ui->t_setupmounth->setText(lst1.at(1));
    ui->t_setupday->setText(lst1.at(2));
  }
  if (exp.six_mounth == "") {
    ui->t_expiredate_day->setText("");
    ui->t_expiredate_mounth->setText("");
    ui->t_expiredate_year->setText("");
  } else {
    ui->t_expiredate_year->setText(exp.two_year);
    ui->t_expiredate_mounth->setText(exp.one_year);
    ui->t_expiredate_day->setText(exp.six_mounth);
  }
}

void
MainWindow::update_query(const QString& id, const QString& name,
                         const QString& family, const QString& address,
                         const QString& tel, const QString& devname,
                         const QString& serial,
                         const QString& setup)
{
    bool retval;
  // update filter
  ExpireData change ;//yy-mm-dd
  change = expire_date(setup);
  // update person
  QString query_up =
    "UPDATE person SET"
    " name='"       + name              + "'," +
    " family='"     + family            + "'," +
    " address='"    + address           + "'," +
    " tel='"        + tel               + "'," +
    " devname='"    + devname           + "'," +
    " serial='"     + serial            + "'," +
    " setup='"      + setup             + "'," +
    " change_six='" + change.six_mounth + "'," +
    " change_one='" + change.one_year   + "'," +
    " change_two='" + change.two_year   + "'"  +
    " WHERE id='"   + id                + "'" + " ;";
  qDebug() << query_up;
  retval =query->exec(query_up);
  CHECK_QURY_ERROR(retval);
  if(retval){
      ui->lbl_status->setText(QString("وضعیت : بروز رسانی انجام شد"));
      search();

  }else{

      ui->lbl_status->setText(QString("وضعیت : بروز رسانی انجام نشد"));
  }
}
void
MainWindow::update_query(const QString& id, const QString& name,
                         const QString& family, const QString& address,
                         const QString& tel, const QString& devname,
                         const QString& serial,
                         const QString& setup,int filtertype)
{
    bool retval;
  // update filter
  ExpireData change ;//yy-mm-dd
  change = expire_date(setup);

  // update person database
  QString query_up="";
  switch (filtertype) {
  case 1:
  case 2:
  case 3://six mounth update + setup update
      query_up =
          "UPDATE person SET"
          " name='"       + name              + "'," +
          " family='"     + family            + "'," +
          " address='"    + address           + "'," +
          " tel='"        + tel               + "'," +
          " devname='"    + devname           + "'," +
          " serial='"     + serial            + "'," +
          " change_six='" + change.six_mounth + "'" +
          " WHERE id='"   + id                + "'" + " ;";
      break;
  case 4:
      query_up =
          "UPDATE person SET"
          " name='"       + name              + "'," +
          " family='"     + family            + "'," +
          " address='"    + address           + "'," +
          " tel='"        + tel               + "'," +
          " devname='"    + devname           + "'," +
          " serial='"     + serial            + "'," +
          " change_two='" + change.two_year   + "'"  +
          " WHERE id='"   + id                + "'" + " ;";
      break;
  case 5:
  case 6:
      query_up =
          "UPDATE person SET"
          " name='"       + name              + "'," +
          " family='"     + family            + "'," +
          " address='"    + address           + "'," +
          " tel='"        + tel               + "'," +
          " devname='"    + devname           + "'," +
          " serial='"     + serial            + "'," +
          " change_one='" + change.one_year   + "'" +
          " WHERE id='"   + id                + "'" + " ;";
      break;
  default://update all
      query_up =
          "UPDATE person SET"
          " name='"       + name              + "'," +
          " family='"     + family            + "'," +
          " address='"    + address           + "'," +
          " tel='"        + tel               + "'," +
          " devname='"    + devname           + "'," +
          " serial='"     + serial            + "'," +
          " setup='"      + setup             + "'," +
          " change_six='" + change.six_mounth + "'," +
          " change_one='" + change.one_year   + "'," +
          " change_two='" + change.two_year   + "'"  +
          " WHERE id='"   + id                + "'" + " ;";

  }
  qDebug() << query_up;
  retval =query->exec(query_up);
  CHECK_QURY_ERROR(retval);
  if(retval){
      ui->lbl_status->setText(QString("وضعیت : تعویض فیلتر انجام شد"));
      search();

  }else{

      ui->lbl_status->setText(QString("وضعیت : تعویض فیلتر انجام نشد"));
  }
}
void
MainWindow::validate(const Date_Validation& dt)
{
  QStringList lst1;
  lst1 = ui->lbl_date->text().split("/");//d m y
  qDebug() << lst1.at(0) << lst1.at(1) << lst1.at(2);
  //                y            m            d

  QVector<int> index;
  QString query_p = "SELECT "
                    "person.id,"
                    "person.name,"
                    "person.family,"
                    "person.address,"
                    "person.tel,"
                    "person.devname,"
                    "person.serial,"
                    "person.setup,"
                    "person.change_six,"
                    "person.change_one,"
                    "person.change_two "
                    "FROM person ;";

  CHECK_QURY_ERROR(query->exec(query_p));
  model->setQuery(*query);

  QString buf[11];

  for (int i = 0; i < model->rowCount(); i++) {
    QSqlRecord rec = model->record(i);
    for (int j = 0; j < rec.count(); j++) {
      buf[j] = rec.value(j).toString();
    }
    QStringList lst2 = buf[8].split("-");
    QStringList lst3 = buf[9].split("-");
    QStringList lst4 = buf[10].split("-");
    // process bufer
    //expire after 6 mounth
    bool res = calc(lst1.at(2), lst1.at(1), lst1.at(0), lst2.at(2), lst2.at(1),
                    lst2.at(0), dt);
    //expire after one year
    bool res1 = calc(lst1.at(2), lst1.at(1), lst1.at(0), lst3.at(2), lst3.at(1),
                    lst3.at(0), dt);

    //expire after two year
    bool res2 = calc(lst1.at(2), lst1.at(1), lst1.at(0), lst4.at(2), lst4.at(1),
                    lst4.at(0), dt);

    if (res || res1 || res2) {
      // store selected rows
      index.push_back(buf[0].toInt());
    }
  } // next query


  QString pac = "";
  for (int ii=0;ii<index.length();ii++) {
    pac += "(person.id = '" +QSTI(index.at(ii))+"')";
    if(ii+1 <index.length())
        pac +=" OR ";
  }
  qDebug() << pac;
  if (pac != "") {
    query_p = "SELECT "
              "person.id,"
              "person.name,"
              "person.family,"
              "person.address,"
              "person.tel,"
              "person.devname,"
              "person.serial,"
              "person.setup "
              "FROM person "
              "WHERE " +
              pac+ " ;";
    if(query->exec(query_p)){

        ui->lbl_status->setText(QString("وضعیت : بررسی تاریخ نصب دستگاه و زمان تعویض آن ها انجام شد"));

    }else
    {
        ui->lbl_status->setText(QString("وضعیت : قادر به پردازش نیست "));
    }
    model->setQuery(*query);
    }
  view->setModel(model);
  view->show();
}

bool
MainWindow::calc(const QString& start_d, const QString& start_m,
                 const QString& start_y, const QString& end_d,
                 const QString& end_m, const QString& end_y,
                 const MainWindow::Date_Validation& _validation)
{
  bool _bvalid = false;
  int sd, sm, sy, ed, em, ey;
  sd = sm = sy = ed = em = ey = 0;
  jalali_to_gregorian(&sy, &sm, &sd, start_y.toInt(), start_m.toInt(),
                      start_d.toInt());

  jalali_to_gregorian(&ey, &em, &ed, end_y.toInt(), end_m.toInt(),
                      end_d.toInt());

  qDebug() << "start : " << start_y << "/" << start_m << "/" << start_d;
  qDebug() << "end : " << end_y << "/" << end_m << "/" << end_d;

  QDate s_date1(sy, sm, sd);
  QDate e_date2(ey, em, ed);

  int distance = (e_date2.year() - s_date1.year()) * 366 - s_date1.dayOfYear() +
                 e_date2.dayOfYear();

  qDebug() << "lenday : " << s_date1.dayOfYear() << "/" << e_date2.dayOfYear();

  qDebug() << "start : " << sy << "/" << sm << "/" << sd;
  qDebug() << "end : " << ey << "/" << em << "/" << ed;
  qDebug() << "distance " << distance;

  if (_validation == Date_Validation::week) {
    if (distance < 9)
      _bvalid = true;
  } else if (_validation == Date_Validation::mounth) {
    if (distance < 32)
      _bvalid = true;
  } else {
    _bvalid = false;
  }

  //(E_y2 - S_y1)*366 - S_d1 +E_d2
  /*
    y2=96/8/5
    d2=240

    y1=95/10/5
    d1=300

    (y2-y1)*366 - d1 +d2
    1*366 - 300 + 240
    */
  return _bvalid;
}

MainWindow::ExpireData MainWindow::expire_date(const QString &setupdate)
{
    ExpireData res;//YY-MM-DD

    QStringList list1=setupdate.split("-");
    int gy,gm,gd;gy=gm=gd=0;
    int jy,jm,jd;jy=jm=jd=0;
    QString y,m,d;y=list1.at(0);m=list1.at(1);d=list1.at(2);

    jalali_to_gregorian(&gy,&gm,&gd,y.toInt(),m.toInt(),d.toInt()) ;

    QDate gdate(gy,gm,gd);
    QDate res1,res2,res3;

    res1 = gdate.addMonths(6);
    res2 = gdate.addYears(1);
    res3 = gdate.addYears(2);

    gregorian_to_jalali(&jy,&jm,&jd,res1.year(),res1.month(),res1.day());
    res.six_mounth = QString("%1-%2-%3").arg(jy).arg(jm).arg(jd);

    gregorian_to_jalali(&jy,&jm,&jd,res2.year(),res2.month(),res2.day());
    res.one_year = QString("%1-%2-%3").arg(jy).arg(jm).arg(jd);

    gregorian_to_jalali(&jy,&jm,&jd,res3.year(),res3.month(),res3.day());
    res.two_year = QString("%1-%2-%3").arg(jy).arg(jm).arg(jd);

    return res;
}


void MainWindow::create_html_from_model()
{
    const QString htmlFileName =
            QString("%1/%2").arg(qApp->applicationDirPath()).arg("myreport.html");
    QFile file(htmlFileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox bx;
        bx.setText(QString("ساخت فایل و پرینت دچار مشکل شده %1").arg(htmlFileName));
        bx.exec();
        return;
    }
    QTextStream out(&file);
    //out.setCodec("UTF-8");

    const int rowCount =view->model()->rowCount() ;
    const int columnCount = view->model()->columnCount();

    out <<  "<html>\n"
            "<head>\n"
            "<!--HTML 4 -->\n"
            "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=UTF-8\">\n"
            "<!--HTML 5 -->\n"
            "<meta charset=\"UTF-8\">\n"
          << QString("<title>Print</title>\n")
          << "<style>"
             "*{"
             "font-family:"
             "\"B titr\","
             "\"B Nazanin\","
             "\"Times New Roman\","
             "\"Times\","
             "\"serif\";\n"
             "font-size : 12px;\n"
             "};\n"
             ".page {"
             "size:8.27in 11.69in;\n"
             "margin:.5in .5in .5in .5in;\n"
             "mso-header-margin:.5in;\n"
             "mso-footer-margin:.5in;\n"
             "mso-paper-source:0;\n"
             "};"
             "</style>"
             "</head>\n"
             "<body class=\"page\" bgcolor=#ffffff link=#5000A0 style=\"float:right;\">\n"
             "<table border=1 cellspacing=0 cellpadding=2>\n";

    // headers
    out << "<tr bgcolor=#f0f0f0>";
    for (int column = columnCount-1; column >=0; column--)
        if (!view->isColumnHidden(column))
            out << QString("<th align=\"center\">%1</th>")
                   .arg(view->model()->headerData(column, Qt::Horizontal).toString());
    out << "</tr>\n";
    file.flush();

    // data table
    for (int row = rowCount-1; row >=0; row--) {
        out << "<tr>";
        for (int column = columnCount-1; column >= 0; column--) {
            if (!view->isColumnHidden(column)) {
                QString data = view->model()->data(view->model()->index(row, column)).toString().simplified();
                out << QString("<td bkcolor=0 align=\"center\" style=\"word-break:break-all;\">%1</td>").arg((!data.isEmpty()) ? data : QString("&nbsp;"));
            }
        }
        out << "</tr>\n";
    }
    out <<  "</table>\n"
            "</body>\n"
            "</html>\n";
    file.close();

    QString h="فایل گزارش جهت چاپ ذخیره شد";h+="\n";h+=htmlFileName;
    QMessageBox msgbox;
    msgbox.setText(h);
    msgbox.exec();
    ui->lbl_status->setText(QString("وضعیت : فایل گزارش جهت چاپ ذخیره شد."));


    QDesktopServices::openUrl(QUrl(htmlFileName, QUrl::TolerantMode));

}

void
MainWindow::alarm()
{
  ++timer_counter;
  timer_counter = timer_counter % 6;
  if (timer_counter == 0) {
    timer->stop();
  }
  if (timer_counter % 2 == 0) {

#define L(name) ui->lbl_##name##_a
    for (auto l : { L(ad), L(family), L(name), L(setup), L(tel) })
      l->setVisible(false);
#undef L

  } else {
    if (ui->t_name->toPlainText()           == "")
      ui->lbl_name_a->setVisible(true);
    if (ui->t_family->toPlainText()         == "")
      ui->lbl_family_a->setVisible(true);
    if (ui->t_address->toPlainText()         == "")
      ui->lbl_ad_a->setVisible(true);
    if (ui->t_tel->toPlainText()             == "")
      ui->lbl_tel_a->setVisible(true);
    if (ui->t_setupday->toPlainText()        == "")
      ui->lbl_setup_a->setVisible(true);
    if (ui->t_setupmounth->toPlainText()     == "")
      ui->lbl_setup_a->setVisible(true);
    if (ui->t_setupyear->toPlainText()      == "")
      ui->lbl_setup_a->setVisible(true);
  }
}

void
MainWindow::search()
{
  QString setup_date = "";
  if (ui->t_setupyear->toPlainText() != "" &&
      ui->t_setupmounth->toPlainText() != "" &&
      ui->t_setupday->toPlainText() != "")
    setup_date = ui->t_setupyear->toPlainText() + "-" +
                 ui->t_setupmounth->toPlainText() + "-" +
                 ui->t_setupday->toPlainText();

  search_query(
    (ui->t_id->toPlainText().toInt() == 0 ? "" : ui->t_id->toPlainText()),
    ui->t_name->toPlainText(), ui->t_family->toPlainText(),
    ui->t_address->toPlainText(), ui->t_tel->toPlainText(),
    ui->t_devicename->toPlainText(), ui->t_deviceserial->toPlainText(),
    setup_date);
}

void
MainWindow::add()
{
  // chack field's
  if (!check_fields()) {
    return;
  }
  QString setup_date = ui->t_setupyear->toPlainText() + "-" +
                       ui->t_setupmounth->toPlainText() + "-" +
                       ui->t_setupday->toPlainText();

  //  QDateConvertor mdate;
  //  DateMiladi mil = mdate.ToMiladi(ui->t_setupyear->toPlainText(),
  //                                  ui->t_setupmounth->toPlainText(),
  //                                  ui->t_setupday->toPlainText());
  //  setup_date = mil.year + "-" + mil.month + "-" + mil.day;

  //add all fillter's
    add_query(ui->t_name->toPlainText(), ui->t_family->toPlainText(),
              ui->t_address->toPlainText(), ui->t_tel->toPlainText(),
              ui->t_devicename->toPlainText(), ui->t_deviceserial->toPlainText(),
              setup_date);
    search();

}

void
MainWindow::on_tableView_3_clicked(const QModelIndex& index)
{
  int id = index.row();
  int col = index.model()->columnCount();
  QStringList lst1;

  // for (int i = 0; i < col; i++)
  //  lst1 << index.model()->index(id, i).data().toString();

  QString id_s = index.model()->index(id, 0).data().toString();
  // set_feilds
  qDebug() << "selected id " << id_s;
  
  //  Qstring qsql_query= "Select * from Person Where id="+ lst1.at(0) + " ;";
  //  query->exec(sqql_query);
  //  QString id_s = QString("%1").arg(id);

  QString  query_p = "SELECT "
                     "person.id,"
                     "person.name,"
                     "person.family,"
                     "person.address,"
                     "person.tel,"
                     "person.devname,"
                     "person.serial,"
                     "person.setup,"
                     "person.change_six,"
                     "person.change_one,"
                     "person.change_two "
                     "FROM person "
                     "WHERE id = ? ;";

  query->prepare(query_p);
  query->addBindValue(id_s);
  CHECK_QURY_ERROR(query->exec());
  query->last();
  QSqlRecord record = query->record();

  ExpireData expdata = { record.value(8).toString(),
                         record.value(9).toString(),
                         record.value(10).toString()};
  // nahve farakhani tabe va por kardan arguman ha az rast be chap
  set_feilds(record.value(0).toString(),         // id
             record.value(1).toString(),       // name
             record.value(2).toString(),     // family
             record.value(3).toString(),    // address
             record.value(4).toString(),        // tel
             record.value(5).toString(),    // devname
             record.value(6).toString(),     // serial
             record.value(7).toString(),      // setup
             expdata); // expire

}

void
MainWindow::on_btn_clear_clicked()
{
  QString empty = "";
  ExpireData empty_data = {"","",""};
  set_feilds(empty, empty, empty, empty, empty, empty, empty, empty,
             empty_data);
}

void
MainWindow::on_btn_delete_clicked()
{
  QString id = ui->t_id->toPlainText();
  //    QString query_s2 = "DELETE FROM filter WHERE person_id= "+id +" ;";
  if (id == "") {
    QMessageBox msgbox(this);
    msgbox.setText("ابتدا یک ردیف را انتخاب کنید سپس بر روی حذف کلیک کنید  ");
    ui->lbl_status->setText(QString("وضعیت : حذف انجام نشد. ابتدا ردیف مورد نظر را انتخاب کنید"));
    msgbox.exec();
    return;
  }
  QString query_s = "DELETE FROM person WHERE id= " + id + ";";


  if(query->exec(query_s)){
      ui->lbl_status->setText(QString("وضعیت : حذف انجام شد"));

  }else{
      CHECK_QURY_ERROR(false);
      ui->lbl_status->setText(QString("وضعیت : حذف انجام نشد . لطفا دوباره سعی کنید"));
  }

  on_btn_clear_clicked();
  search();
}

void
MainWindow::on_pushButton_7_clicked()
{
  if (ui->t_id->toPlainText() == "") {
    QMessageBox msg(this);
    msg.setText("ابتدا ردیف مورد نظر را انتخاب کنید");
    ui->lbl_status->setText(QString("وضعیت : بروزرسانی انجام نشد. ابتدا ردیف مورد نظر را انتخاب کنید"));
    msg.exec();
    return;
  }
  QString stp = ui->t_setupyear->toPlainText() + "-" +
                ui->t_setupmounth->toPlainText() + "-" +
                ui->t_setupday->toPlainText();
  update_query(ui->t_id->toPlainText(), ui->t_name->toPlainText(),
               ui->t_family->toPlainText(), ui->t_address->toPlainText(),
               ui->t_tel->toPlainText(), ui->t_devicename->toPlainText(),
               ui->t_deviceserial->toPlainText(),
               stp);
}

bool
MainWindow::isNumeric_text(const QString& c)
{
  bool ok;
  c.toInt(&ok);
  return ok;
}

void
MainWindow::on_TextChange_setupday()
{
  QString test = ui->t_setupday->toPlainText();
  if (test == "") {
    ui->t_setupday->setStyleSheet("background-color:white;");
    valid_day_check = false;
    return;
  }
  if (!isNumeric_text(test) || (ui->t_setupday->toPlainText().length() > 2)) {
    ui->t_setupday->setStyleSheet("background-color:red;");
    valid_day_check = false;

  } else if (test.toInt() > 31) {
    ui->t_setupday->setStyleSheet("background-color:red;");
    valid_day_check = false;

  } else {
    ui->t_setupday->setStyleSheet("background-color:white;");
    valid_day_check = true;
  }
}

void
MainWindow::on_TextChange_setupmounth()
{
  QString test = ui->t_setupmounth->toPlainText();
  if (test == "") {
    ui->t_setupmounth->setStyleSheet("background-color:white;");
    valid_mount_check = false;
    return;
  }
  if (!isNumeric_text(test) ||
      (ui->t_setupmounth->toPlainText().length() > 2)) {
    ui->t_setupmounth->setStyleSheet("background-color:red;");
    valid_mount_check = false;
  } else if (test.toInt() > 12) {
    ui->t_setupmounth->setStyleSheet("background-color:red;");
    valid_mount_check = false;

  } else {
    ui->t_setupmounth->setStyleSheet("background-color:white;");
    valid_mount_check = true;
  }
}

void
MainWindow::on_TextChange_setupyear()
{
  QString test = ui->t_setupyear->toPlainText();
  if (test == "") {
    ui->t_setupyear->setStyleSheet("background-color:white;");
    valid_year_check = false;
    return;
  }
  if (!isNumeric_text(test) || (test.length() != 4)) {
    ui->t_setupyear->setStyleSheet("background-color:red;");
    valid_year_check = false;
  } else {
    ui->t_setupyear->setStyleSheet("background-color:white;");
    valid_year_check = true;
  }
}


void
MainWindow::on_pushButton_9_clicked()
{
  validate(Date_Validation::mounth);
}

void MainWindow::on_btn_print_clicked()
{
    create_html_from_model();
}

void MainWindow::six_mounth_update()
{
    //id check
    if (ui->t_id->toPlainText() == "") {
      QMessageBox msg(this);
      msg.setText("ابتدا ردیف مورد نظر را انتخاب کنید");
      ui->lbl_status->setText(QString("وضعیت : بروزرسانی انجام نشد. ابتدا ردیف مورد نظر را انتخاب کنید"));
      msg.exec();
      return;
    }
    //get date from system
    QStringList lst1;
    lst1 = ui->lbl_date->text().split("/");//d m y
    qDebug() << lst1.at(0) << lst1.at(1) << lst1.at(2);
    //                y            m            d

    QString stp = lst1.at(0) + "-" +
                  lst1.at(1) + "-" +
                  lst1.at(2);

    update_query(ui->t_id->toPlainText(), ui->t_name->toPlainText(),
                 ui->t_family->toPlainText(), ui->t_address->toPlainText(),
                 ui->t_tel->toPlainText(), ui->t_devicename->toPlainText(),
                 ui->t_deviceserial->toPlainText(),
                 stp,1);
}

void MainWindow::one_year_update()
{
    if (ui->t_id->toPlainText() == "") {
      QMessageBox msg(this);
      msg.setText("ابتدا ردیف مورد نظر را انتخاب کنید");
      ui->lbl_status->setText(QString("وضعیت : بروزرسانی انجام نشد. ابتدا ردیف مورد نظر را انتخاب کنید"));
      msg.exec();
      return;
    }
    //get date from system
    QStringList lst1;
    lst1 = ui->lbl_date->text().split("/");//d m y
    qDebug() << lst1.at(0) << lst1.at(1) << lst1.at(2);
    //                y            m            d

    QString stp = lst1.at(0) + "-" +
                  lst1.at(1) + "-" +
                  lst1.at(2);

    update_query(ui->t_id->toPlainText(), ui->t_name->toPlainText(),
                 ui->t_family->toPlainText(), ui->t_address->toPlainText(),
                 ui->t_tel->toPlainText(), ui->t_devicename->toPlainText(),
                 ui->t_deviceserial->toPlainText(),
                 stp,5);
}

void MainWindow::two_year_update()
{
    if (ui->t_id->toPlainText() == "") {
      QMessageBox msg(this);
      msg.setText("ابتدا ردیف مورد نظر را انتخاب کنید");
      ui->lbl_status->setText(QString("وضعیت : بروزرسانی انجام نشد. ابتدا ردیف مورد نظر را انتخاب کنید"));
      msg.exec();
      return;
    }
    //get date from system
    QStringList lst1;
    lst1 = ui->lbl_date->text().split("/");//d m y
    qDebug() << lst1.at(0) << lst1.at(1) << lst1.at(2);
    //                y            m            d

    QString stp = lst1.at(0) + "-" +
                  lst1.at(1) + "-" +
                  lst1.at(2);

    update_query(ui->t_id->toPlainText(), ui->t_name->toPlainText(),
                 ui->t_family->toPlainText(), ui->t_address->toPlainText(),
                 ui->t_tel->toPlainText(), ui->t_devicename->toPlainText(),
                 ui->t_deviceserial->toPlainText(),
                 stp,4);
}

