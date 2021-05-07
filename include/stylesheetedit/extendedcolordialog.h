#ifndef SVGCOLORNAMEDIALOG_H
#define SVGCOLORNAMEDIALOG_H

#include <QAction>
#include <QApplication>
#include <QBuffer>
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

  void setPrimaryColor(const QColor& color, const QString& name);
  void setSecondaryColor(const QColor& color, const QString& name);

  static const QString DISPLAYLABELRIGHT;
  static const QString DISPLAYLABELLEFT;
  static const QString DISPLAYBORDER;

  QColor color() const;
  QColor dropColor() const;
  QString name() const;
  QString dropName() const;

signals:
  void primaryColorChanged(const QColor& color, const QString& name);
  void secondaryColorChanged(const QColor& color, const QString& name);

protected:
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;

  QString colorToStyle(const QColor& color, Side side = Right);

private:
  QLabel* m_left;
  QLabel* m_right;
  QColor m_color;
  QString m_name;
  QColor m_dropColor;
  QString m_dropName;
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

struct ColorDragData
{
  int r;
  int g;
  int b;
  QString name;
};
Q_DECLARE_METATYPE(ColorDragData)

QDataStream&
operator<<(QDataStream& out, const ColorDragData& a);
QDataStream&
operator>>(QDataStream& in, ColorDragData& a);

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
               bool x11,
               const QString& back,
               const QString& fore = QString());

protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragMoveEvent(QDragMoveEvent*) override;

private:
  QPoint m_dragStartPosition;
  QColor m_color;
  QString m_name;
  ColorDragModel* m_model;
  QSize m_size;
  QPixmap m_pixmap;
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
  ExtendedColorDialog(const QString& initialColor, QWidget* parent = nullptr);
  ExtendedColorDialog(const QColor& initialColor,
                      const QColor& secondaryColor,
                      QWidget* parent = nullptr);
  ExtendedColorDialog(const QString& initialColor,
                      const QString& secondaryColor,
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
  void setPrimaryColor(const QColor& color, const QString& name = QString());
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
  void setSecondaryColor(const QColor& color, const QString& name = QString());

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
  //! Returns the secondary color as a stylesheet name string.
  //!
  //! If no color has been selected then the color will be tested
  //! against all possible X11 name strings and if one matches this
  //! name will be returned.
  //! If 'Cancel' has been pressed then an empty string is returned.
  QString secondaryName() const;

  //! Returns the selected color as a #ffffff style string.
  //!
  //! If alpha is specified a #ffffffff form will be returned. The value of
  //! alpha must it must be between 0 (transparent)
  //! and 100 (opaque), the default, otherwise it will be ignored.
  //! If no color has been selected or  'Cancel' has been pressed then
  //! an empty string is returned.
  QString hash(int alpha = false) const;

  static QString svgOrX11Name(const QColor& color);
  static QColor svgOrX11Color(const QString& initialColor);

signals:
  //! This signal is emitted whenever the primary color changes in the dialog.
  //! The current color is specified by color.
  void primaryColorChanged(const QColor& color, const QString& name);
  //! This signal is emitted whenever the secondary color changes in the dialog.
  //! The current color is specified by color.
  void secondaryColorChanged(const QColor& color, const QString& name);


protected:
  QSize sizeHint() const override;

private:
  QTabWidget* m_tabs;
  QColor m_color;
  QColor m_dropColor;
  QString m_name;
  QString m_dropName;
  QColorDialog* m_colorDlg;
  ColorDropDisplay* m_display;
  Tabs m_currentTab;

//  void acceptColor();
  void acceptChanges();
  void initGui();
  ColorDragTable* initSvgFrame1();
  ColorDragTable* initSvgFrame2();
  ColorDragTable* initX11ColorFrame1();
  ColorDragTable* initX11ColorFrame2();
  ColorDragTable* initX11MonoFrame();
  QDialogButtonBox* createBtnBox();
  ColorDropDisplay* createColorDisplay();
  ColorDragTable* createColorTable();
  void colorClicked(const QModelIndex& index);
  void primaryColorHasChanged(const QColor& color, const QString &name);
  void dialogColorHasChanged(const QColor& color);
  void secondaryColorHasChanged(const QColor& color, const QString &name);

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
