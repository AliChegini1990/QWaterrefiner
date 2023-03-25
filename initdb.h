#ifndef INITDB_H
#define INITDB_H
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QTableView>
#include <QtSql>
#include <memory>


class SqlHelper
{

public:
  SqlHelper(QTableView* view)
  {
    InitDB();
    InitModel(view);
  }

  ~SqlHelper() { Close(); }
  const std::unique_ptr<QSqlDatabase> GetDatabase()
  {
    std::unique_ptr<QSqlDatabase> pSqlDatabase{ &db };
    return pSqlDatabase;
  }

private:
  void InitDB()
  {
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
    if (new_db)
      CreateTables(); // create tables
  }
  void InitModel(QTableView* view)
  {

    CHECK_QURY_ERROR(
      query->exec("insert into persion (name,family,address,tel) "
                  "values('علی','ch','add','0282')"));
    CHECK_QURY_ERROR(query->exec(
      "insert into device (id,name,serial) values(2,'dev1','9963')"));
    model_persion = new QSqlRelationalTableModel(nullptr, db);
    model_persion->setTable("persion");
    model_persion->setEditStrategy(
      QSqlTableModel::EditStrategy::OnManualSubmit);
    model_persion->setRelation(0, QSqlRelation("device", "id", "name"));
    // model_persion->setJoinMode(QSqlRelationalTableModel::InnerJoin);

    model_persion->setHeaderData(0, Qt::Horizontal, QObject::tr("ردیف"));
    model_persion->setHeaderData(1, Qt::Horizontal, QObject::tr("نام"));
    model_persion->setHeaderData(2, Qt::Horizontal,
                                 QObject::tr("نام خانوادگی"));
    model_persion->setHeaderData(3, Qt::Horizontal, QObject::tr("آدرس"));
    model_persion->setHeaderData(4, Qt::Horizontal, QObject::tr("تلفن"));
    model_persion->setHeaderData(5, Qt::Horizontal, QObject::tr("نام دستگاه"));
    model_persion->setHeaderData(6, Qt::Horizontal, QObject::tr("سریال"));
    model_persion->setHeaderData(7, Qt::Horizontal, QObject::tr("نوع فیلتر"));
    model_persion->setHeaderData(8, Qt::Horizontal, QObject::tr("تاریخ نصب"));
    model_persion->setHeaderData(9, Qt::Horizontal, QObject::tr("تاریخ تعویض"));

    // model_persion.setFilter("ORDERBY id ASC");//LIMIT 10
    model_persion->select();
    qDebug() << model_persion->lastError().text();

    view->setModel(model_persion);
    view->setItemDelegate(new QSqlRelationalDelegate(view));
    view->setWindowTitle("title");
    view->show();
  }
  //    void createView(QTableView * view)
  //    {
  //        //view->setModel(model);
  //        view->setItemDelegate(new QSqlRelationalDelegate(view));
  //        view->setWindowTitle("title");
  //    }
  bool CreateDbIfNotExist(const QString& path_)
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
  void Close()
  {
    db.close();
    delete query;
  }
  void inline CHECK_QURY_ERROR(const bool& query_exec)
  {
    if (!query_exec) {
      qDebug() << query->lastError().text();
      qDebug() << __func__ << __FILE__ << __LINE__;
      exit(0);
    }
  }
  void inline CreateTables()
  {
    QString persion_query = "CREATE TABLE persion( \
                id INTEGER primary key autoincrement,\
                name string,\
                family string,\
                address string,\
                tel string);";
    QString device_query = "CREATE TABLE device( \
                id INT,\
                name string,\
                serial string,\
                FOREIGN KEY(id) REFERENCES persion(id) );";

    QString device_filter = "CREATE TABLE filter( \
                id INT,\
                filtertype INT,\
                setup DATE,\
                change DATE,\
                FOREIGN KEY(id) REFERENCES device(id) );";

    CHECK_QURY_ERROR(query->exec(persion_query));
    CHECK_QURY_ERROR(query->exec(device_query));
    CHECK_QURY_ERROR(query->exec(device_filter));
  }
  QSqlRelationalTableModel* model_persion;
  QSqlDatabase db;
  QSqlQuery* query;
  bool new_db;
};

#endif // INITDB_H
