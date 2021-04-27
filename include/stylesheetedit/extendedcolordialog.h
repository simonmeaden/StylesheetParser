#ifndef SVGCOLORNAMEDIALOG_H
#define SVGCOLORNAMEDIALOG_H

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QColorDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDrag>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QObject>
#include <QRadioButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QPainter>

#include <bits/functional_hash.h>

class ColorDropLabel : public QWidget
{
  Q_OBJECT
public:
  ColorDropLabel(QWidget* parent = nullptr);

  void setPrimaryColor(const QColor& color);
  void setSecondaryColor(const QColor& color);

  static const QString DISPLAYLABELSTYLE;

protected:
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;
  void paintEvent(QPaintEvent *) override
  {
      QStyleOption opt;
      opt.init(this);
      QPainter p(this);
      style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
  }

  QString colorToString(QColor color);

private:
  QLabel *m_left, *m_right;
  QColor m_color, m_dropColor;
};

class ColorDragWidget : public QTableWidget
{
  Q_OBJECT
public:
  ColorDragWidget(QWidget* parent = nullptr);

protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;

private:
  QPoint m_dragStartPosition;
  QColor m_color;
};

class ExtendedColorDialog : public QDialog
{
  Q_OBJECT
public:
  ExtendedColorDialog(QWidget* parent = nullptr);
  ExtendedColorDialog(const QColor& initialColor, QWidget* parent = nullptr);

  //! Returns the selected color as a QColor object.
  //!
  //! If no color has been selected or  'Cancel' has been pressed then the
  //! color will be Invalid.
  //! \sa rgb()
  //! \sa hsv()
  //! \sa hsl()
  //! \sa name()
  //! \sa hash()
  QColor color() const;

  //! Returns the selected color as a stylesheet rgb() string.
  //!
  //! If alpha is specified an rgba() form will be returned. The value of
  //! alpha must it must be between 0 (transparent)
  //! and 100 (opaque), the default, otherwise it will be ignored.
  //! If no color has been selected or  'Cancel' has been pressed then
  //! an empty string is returned.
  QString rgb(int alpha = 100) const;

  //! Returns the selected color as a stylesheet hsv() string.
  //!
  //! If alpha is specified an hsva() form will be returned. The value of
  //! alpha must it must be between 0 (transparent)
  //! and 100 (opaque), the default, otherwise it will be ignored.
  //! If no color has been selected or  'Cancel' has been pressed then
  //! an empty string is returned.
  QString hsv(int alpha = false) const;

  //! Returns the selected color as a stylesheet hsl() string.
  //!
  //! If alpha is specified an hsla() form will be returned. The value of
  //! alpha must it must be between 0 (transparent)
  //! and 100 (opaque), the default, otherwise it will be ignored.
  //! If no color has been selected or  'Cancel' has been pressed then
  //! an empty string is returned.
  QString hsl(int alpha = false) const;

  //! Returns the selected color as a stylesheet name string.
  //!
  //! If no color has been selected then the color will be tested
  //! against all possible X11 name strings and if one matches this
  //! name will be returned.
  //! If 'Cancel' has been pressed then an empty string is returned.
  QString name() const;

  //! Returns the selected color as a #ffffff style string.
  //!
  //! If alpha is specified a #ffffffff form will be returned. The value of
  //! alpha must it must be between 0 (transparent)
  //! and 100 (opaque), the default, otherwise it will be ignored.
  //! If no color has been selected or  'Cancel' has been pressed then
  //! an empty string is returned.
  QString hash(int alpha = false) const;

signals:
  //! This signal is emitted just after the user has clicked OK to
  //! select a color to use. The chosen color is specified by color.
  void colorSelected(const QColor& color);

  //! This signal is emitted whenever the current color changes in the dialog.
  //! The current color is specified by color.
  //! \note Notifier signal for property currentColor.
  void currentColorChanged(const QColor& color);

protected:
  QSize sizeHint() const override;

private:
  QTabWidget* m_tabs;
  QTableWidget *m_svgTable, *m_x11ColorTbl1, *m_x11ColorTbl2, *m_x11MonoTbl;
  QColor m_color;
  QString m_name;
  QColorDialog* m_colorDlg;
  ColorDropLabel *m_col1SelectionLbl, *m_col2SelectionLbl, *m_monoSelectionLbl,
    *m_svgSelectionLbl;

  void acceptColor();
  void acceptStandardColor();
  void initGui();
  QFrame* initSvgFrame();
  QFrame* initX11ColorFrame1();
  QFrame* initX11ColorFrame2();
  QFrame* initX11MonoFrame();
  QDialogButtonBox* createBtnBox();
  ColorDropLabel *createColorDisplay();
  QTableWidgetItem* getSvgItem(const QString& back,
                               const QString& fore = QString("black"));
  QTableWidgetItem* getX11Color1Item(const QString& back,
                                     const QString& fore = QString("black"));
  QTableWidgetItem* getX11Color2Item(const QString& back,
                                     const QString& fore = QString("black"));
  QTableWidgetItem* getX11MonoItem(const QString& back,
                                   const QString& fore = QString("black"));
  void selectSvgColor(int row, int column);
  void selectX11_1Color(int row, int column);
  void selectX11_2Color(int row, int column);
  void selectX11_Mono(int row, int column);

  static const QString HASHACOLOR;
  static const QString HASHCOLOR;
  static const QString RGBCOLOR;
  static const QString RGBACOLOR;
  static const QString HSLCOLOR;
  static const QString HSLACOLOR;
  static const QString HSVCOLOR;
  static const QString HSVACOLOR;
};

#endif // SVGCOLORNAMEDIALOG_H
