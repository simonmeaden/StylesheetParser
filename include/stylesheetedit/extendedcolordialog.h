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
#include <QPainter>
#include <QRadioButton>
#include <QStringList>
#include <QTabWidget>
#include <QTableWidget>

#include <bits/functional_hash.h>

class ColorDropDisplay : public QFrame
{
  Q_OBJECT
  enum Side
  {
    Left,
    Right,
  };

public:
  ColorDropDisplay(const QColor& color,
                 const QColor& dropColor,
                 QWidget* parent = nullptr);

  void setCurrentColor(const QColor& color);
  void setSecondaryColor(const QColor& color);

  static const QString DISPLAYLABELRIGHT;
  static const QString DISPLAYLABELLEFT;
  static const QString DISPLAYBORDER;

  QColor color() const;
  QColor dropColor() const;

protected:
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;

  QString colorToStyle(const QColor& color, Side side = Right);

private:
  QLabel* m_left;
  QLabel* m_right;
  QColor m_color;
  QColor m_dropColor;
  bool m_colorSet, m_dropColorSet;
};

class ColorDragModel : public QAbstractTableModel
{
  Q_OBJECT

  struct Data
  {
    Data(const QString& n, const QColor& f, const QColor& b)
      : name(n)
      , fore(f)
      , back(b)
    {}
    QString name;
    QColor fore;
    QColor back;
  };
  typedef Data* Row;
  typedef Row* Column;

public:
  ColorDragModel(int rows, int columns);
  ~ColorDragModel();

  Qt::ItemFlags flags(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& = QModelIndex()) const override;
  int columnCount(const QModelIndex& = QModelIndex()) const override;
  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  QVariant headerData(int,
                      Qt::Orientation,
                      int = Qt::DisplayRole) const override;
  void setColorData(int row,
                    int column,
                    const QString& name,
                    const QColor& back,
                    const QColor& fore);
  QModelIndex index(int row,
                    int column,
                    const QModelIndex& = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex&) const override;

private:
  Column* m_data;
  int m_rows = 0;
  int m_columns = 0;
};

class ColorDragTable : public QTableView
{
  Q_OBJECT
public:
  ColorDragTable(int rows, int columns, QWidget* parent = nullptr);

  QString name(const QModelIndex& index);
  QColor foreground(const QModelIndex& index);
  QColor background(const QModelIndex& index);
  void setData(int row,
               int column,
               const QString& back,
               const QString& fore = QString());

  void setLabel(ColorDropDisplay* label);
  ColorDropDisplay* label() const;

protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragMoveEvent(QDragMoveEvent*) override;

private:
  QPoint m_dragStartPosition;
  QColor m_color;
  ColorDragModel* m_model;
  QSize m_size;
  QPixmap m_pixmap;
  ColorDropDisplay* m_label;
};

class ColorDragFrame : public QFrame
{
  Q_OBJECT
public:
  ColorDragFrame(QWidget* parent)
    : QFrame(parent) {

  }

  ColorDragTable *table() const;
  void setTable(ColorDragTable *table);

  ColorDropDisplay *getDisplay() const;
  void setDisplay(ColorDropDisplay *value);

  QGridLayout*layout() const;

private:
  ColorDragTable* m_table;
  ColorDropDisplay* display;
};

class ExtendedColorDialog : public QDialog
{
  Q_OBJECT

  enum Tabs
  {
    ColorDialog,
    SvgTab,
    X11Color1Tab,
    X11Color2Tab,
    X11MonoTab,
  };

public:
  ExtendedColorDialog(QWidget* parent = nullptr);
  ExtendedColorDialog(const QColor& initialColor, QWidget* parent = nullptr);
  ExtendedColorDialog(const QColor& initialColor,
                      const QColor& secondaryColor,
                      QWidget* parent = nullptr);

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
  //! Sets the initial color as a QColor object.
  //! \sa setSecondaryColor()
  void setColor(const QColor& color);
  //! Returns the selected secondary color as a QColor object.
  //!
  //! The secondary color allows the user to display and
  //! compare two colors.
  //! \sa setSecondaryColor()
  QColor secondaryColor();
  //! Sets the secondary color as a QColor object.
  //!
  //! The secondary color allows the user to display and
  //! compare two colors.
  //! \sa setColor()
  void setSecondaryColor(const QColor& color);

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
  //  ColorDragTable *m_svgTable, *m_x11Color1Tbl, *m_x11Color2Tbl,
  //  *m_x11MonoTbl;
  QColor m_color, m_dropColor;
  QString m_name;
  QColorDialog* m_colorDlg;
  //  ColorDropLabel *m_x11Color1Lbl, *m_x11Color2Lbl, *m_x11MonoLbl,
  //    *m_svgColor1Lbl;
  Tabs m_currentTab;

  void acceptColor();
  void acceptStandardColor();
  void initGui();
  QFrame* initSvgFrame();
  QFrame* initX11ColorFrame1();
  QFrame* initX11ColorFrame2();
  QFrame* initX11MonoFrame();
  QDialogButtonBox* createBtnBox();
  ColorDropDisplay* createColorDisplay();
  ColorDragTable* createColorTable(ColorDragFrame *frame);
  //  void setSvgItem(int row,
  //                  int column,
  //                  const QString& back,
  //                  const QString& fore = QString("black"));
  //  void setX11Item(ColorDragTable* table,
  //                  int row,
  //                  int column,
  //                  const QString& back,
  //                  const QString& fore = QString(Qt::black));
  void colorClicked(const QModelIndex& index);
  //  void x11Color1Clicked(const QModelIndex& index);
  //  void selectX11_2Color(const QModelIndex& index);
  //  void selectX11_Mono(const QModelIndex& index);

  void tabChanged(int index);

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
