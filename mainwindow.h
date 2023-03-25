#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>
#include <QFile>
#include <QIntValidator>
#include <QMainWindow>
#include <QMessageBox>
#include <QTableView>
#include <QtSql>
#include <initdb.h>
#include <QDate>
#include <QDesktopServices>


//QString to Int
#define QSTI(x) QString("%1").arg(x)

namespace Ui {
class MainWindow;
}

class MyModel:public QSqlQueryModel
{
    Q_OBJECT

public:
    MyModel(QObject *parent=nullptr):QSqlQueryModel(parent)
    {
    }
private:

};

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();

private:
  enum Date_Validation_
  {
    week,
    mounth
  };
  enum Bool_Validation_
  {
    bool_week,
    bool_mounth
  };
  typedef enum Bool_Validation_ Bool_Validation;
  typedef enum Date_Validation_ Date_Validation; // yek hafte , yek mah

  struct tagexpire
  {
      QString six_mounth;
      QString one_year;
      QString two_year;
  };
  typedef tagexpire ExpireData;

  void InitDB();
  void InitModel(QTableView* _view);
  bool CreateDbIfNotExist(const QString &path_);
  void Close_DB(); // close db , free memory
  void inline CHECK_QURY_ERROR_F(const bool& query_exec, const char* func,
                                 const char* file,
                                 int line); // show query error
  void inline CreateTables();
  //void search_query(const dataPack &pack); // search
  void search_query(const QString &pid, const QString &name,
                    const QString &family, const QString &address,
                    const QString &tel, const QString &devname,
                    const QString &serial,const QString &setup); // search

  void add_query(const QString &name, const QString& family,
                 const QString &address, const QString& tel,
                 const QString &devname, const QString& serial,
                 const QString &setup); // add query to database

  bool check_fields(); // check UI feilds for validation

  void set_feilds(const QString &pid, const QString &name,
                  const QString &family, const QString &address,
                  const QString &tel, const QString &devicename,
                  const QString &serial, const QString &setup,
                  const ExpireData exp); // set UI feilds

  void update_query(const QString &id, const QString &name,
                    const QString &family, const QString &address,
                    const QString &tel, const QString &devname,
                    const QString &serial,
                    const QString &setup); // add query to database

  void update_query(const QString& id, const QString& name,
                   const QString& family, const QString& address,
                   const QString& tel, const QString& devname,
                   const QString& serial,
                   const QString& setup,int filtertype);


  void validate(const MainWindow::Date_Validation &dt);


  bool calc(const QString &start_d, const QString &start_m,
            const QString &start_y, const QString &end_d,
            const QString &end_m, const QString &end_y,
            const Date_Validation &_validation);



  void gregorian_to_jalali(int *j_y, int *j_m, int *j_d,
                           int  g_y, int  g_m, int  g_d)
  {
     int gy, gm, gd;
     int jy, jm, jd;
     long g_day_no, j_day_no;
     int j_np;

     int i;

     gy = g_y-1600;
     gm = g_m-1;
     gd = g_d-1;

     g_day_no = 365*gy+(gy+3)/4-(gy+99)/100+(gy+399)/400;
     for (i=0;i<gm;++i)
        g_day_no += g_days_in_month[i];
     if (gm>1 && ((gy%4==0 && gy%100!=0) || (gy%400==0)))
        /* leap and after Feb */
        ++g_day_no;
     g_day_no += gd;

     j_day_no = g_day_no-79;

     j_np = j_day_no / 12053;
     j_day_no %= 12053;

     jy = 979+33*j_np+4*(j_day_no/1461);
     j_day_no %= 1461;

     if (j_day_no >= 366) {
        jy += (j_day_no-1)/365;
        j_day_no = (j_day_no-1)%365;
     }

     for (i = 0; i < 11 && j_day_no >= j_days_in_month[i]; ++i) {
        j_day_no -= j_days_in_month[i];
     }
     jm = i+1;
     jd = j_day_no+1;
     *j_y = jy;
     *j_m = jm;
     *j_d = jd;
  }

  void jalali_to_gregorian(int *g_y, int *g_m, int *g_d,
                           int  j_y, int  j_m, int  j_d)
  {
     int gy, gm, gd;
     int jy, jm, jd;
     long g_day_no, j_day_no;
     int leap;

     int i;

     jy = j_y-979;
     jm = j_m-1;
     jd = j_d-1;

     j_day_no = 365*jy + (jy/33)*8 + (jy%33+3)/4;
     for (i=0; i < jm; ++i)
        j_day_no += j_days_in_month[i];

     j_day_no += jd;

     g_day_no = j_day_no+79;

     gy = 1600 + 400*(g_day_no/146097); /* 146097 = 365*400 + 400/4 - 400/100 + 400/400 */
     g_day_no = g_day_no % 146097;

     leap = 1;
     if (g_day_no >= 36525) /* 36525 = 365*100 + 100/4 */
     {
        g_day_no--;
        gy += 100*(g_day_no/36524); /* 36524 = 365*100 + 100/4 - 100/100 */
        g_day_no = g_day_no % 36524;

        if (g_day_no >= 365)
           g_day_no++;
        else
           leap = 0;
     }

     gy += 4*(g_day_no/1461); /* 1461 = 365*4 + 4/4 */
     g_day_no %= 1461;

     if (g_day_no >= 366) {
        leap = 0;

        g_day_no--;
        gy += g_day_no/365;
        g_day_no = g_day_no % 365;
     }

     for (i = 0; g_day_no >= g_days_in_month[i] + (i == 1 && leap); i++)
        g_day_no -= g_days_in_month[i] + (i == 1 && leap);
     gm = i+1;
     gd = g_day_no+1;

     *g_y = gy;
     *g_m = gm;
     *g_d = gd;
  }


  ExpireData expire_date(const QString &setupdate/*Y-M-D*/);
  void create_html_from_model();
private:
  Ui::MainWindow* ui;
  QSqlDatabase db;
  MyModel* model;
  QSqlQuery* query;
  QTableView* view; // container
  bool new_db;
  QTimer* timer;
  int timer_counter;
  bool valid_day_check;
  bool valid_mount_check;
  bool valid_year_check;
  int g_days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int j_days_in_month[12] = {31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29};
  const char *j_month_name[13] = {"",
                                  "Farvardin", "Ordibehesht", "Khordad",
                                  "Tir", "Mordad", "Shahrivar",
                                  "Mehr", "Aban", "Azar",
                                  "Dey", "Bahman", "Esfand"};

public slots:
  void search(); // on search button click
  void add();    // on add button click
  void alarm(); // if error occur in check_feild function then timer will enable
                // and alarm fire;

private slots:
  void six_mounth_update();
  void one_year_update();
  void two_year_update();

  void on_tableView_3_clicked(const QModelIndex& index);
  void on_btn_clear_clicked();
  void on_btn_delete_clicked();
  void on_pushButton_7_clicked();
  bool isNumeric_text(const QString& c);
  void on_pushButton_9_clicked();

  void on_TextChange_setupday();
  void on_TextChange_setupmounth();
  void on_TextChange_setupyear();

  void on_btn_print_clicked();
};

#endif // MAINWINDOW_H
