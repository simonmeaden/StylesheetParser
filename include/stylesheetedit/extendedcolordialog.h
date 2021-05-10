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
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QObject>
#include <QPainter>
#include <QRadioButton>
#include <QStringList>
#include <QTabWidget>
#include <QTableWidget>
#include <QtMath>

#include <bits/functional_hash.h>

enum ColorType
{
  Primary,
  Secondary,
};

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
  void setPrimaryTextColor(const QColor& color, const QString& name);
  void setSecondaryColor(const QColor& color, const QString& name);
  void setSecondaryTextColor(const QColor& color, const QString& name);

  static const QString DISPLAYLABELRIGHT;
  static const QString DISPLAYLABELLEFT;
  static const QString DISPLAYBORDER;

  QColor primaryColor() const;
  QColor secondaryColor() const;
  QString primaryName() const;
  QString secondaryName() const;

signals:
  void primaryColorChanged(const QColor& color, const QString& name);
  void primaryTextColorChanged(const QColor& color, const QString& name);
  void secondaryColorChanged(const QColor& color, const QString& name);
  void secondaryTextColorChanged(const QColor& color, const QString& name);

protected:
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;

private:
  QLabel* m_left;
  QLabel* m_right;
  QColor m_primaryColor;
  QColor m_primaryTextColor;
  QString m_primaryName;
  QString m_primaryTextName;
  QColor m_secondaryColor;
  QColor m_secondaryTextColor;
  QString m_secondaryName;
  QString m_secondaryTextName;
  bool m_colorSet, m_dropColorSet;

  QString calculateTextString(ColorType type);
  QString primaryColorToStyle();
  QString secondaryColorToStyle();
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
  void setData(int row, int column, bool x11, const QString& back);

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

// clang-format off
/*!
   \file extendedcolordialog.h extendedcolordialog.cpp
   \class ExtendedColorDialog extendedcolordialog.h
   \brief The ExtendedColorDialog class provides an extension of the standard
          QColorDialog class that includes selection pages for both the SVG color name set and the
          X11 color name set.
   \since 5.7.0
   \license The MIT License
   \copyright © 2020 - 2021 Simon Meaden. All rights reserved.


*/
// clang-format on
class ExtendedColorDialog : public QDialog
{
  Q_OBJECT

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

  //! Returns the primary (left) color as a QColor object.
  //!
  //! This would be the 'background' property in a stylesheet.
  //! type must be either ColorType::Primary for the left half of the display,
  //! or ColorType::Secondary for the right half.
  //! If no color has been selected or  'Cancel' has been pressed then the
  //! color will be Invalid.
  //! \sa rgb()
  //! \sa hsv()
  //! \sa hsl()
  //! \sa name()
  //! \sa hash()
  QColor color(ColorType type) const;
  //! Returns the primary (left) text color as a QColor object.
  //!
  //! This would be the 'color' property in a stylesheet.
  //! If no color has been selected or  'Cancel' has been pressed then the
  //! color will be Invalid.
  //! \sa rgb()
  //! \sa hsv()
  //! \sa hsl()
  //! \sa name()
  //! \sa hash()
  QColor textColor(ColorType type) const;
  //! Sets the initial primary (left) background color.
  //! If this color has a matching SVG or X11 name this will be set
  //! automatically. \sa setSecondaryColor()
  void setColor(ColorType type,
                const QColor& color,
                const QString& name = QString());
  //! Sets the initial primary (left) text color.
  //! \sa setSecondaryColor()
  void setTextColor(ColorType type,
                    const QColor& color,
                    const QString& name = QString());

  //! Returns the selected color as a stylesheet rgb() string.
  //!
  //! The type should be either ColorTye::Primary or ColorType::Secondary,
  //! Primary by default. If alpha is specified an rgba() form will be returned.
  //! The value of alpha must it must be between 0 (transparent) and 100
  //! (opaque), the default, otherwise it will be ignored. If no color has been
  //! selected or  'Cancel' has been pressed then an empty string is returned.
  QString rgb(ColorType type, int alpha = -1) const;

  //! Returns the selected color as a stylesheet hsv() string.
  //!
  //! If alpha is specified an hsva() form will be returned. The value of
  //! alpha must it must be between 0 (transparent)
  //! and 100 (opaque), the default, otherwise it will be ignored.
  //! If no color has been selected or  'Cancel' has been pressed then
  //! an empty string is returned.
  QString hsv(ColorType type, int alpha = 100) const;

  //! Returns the selected color as a stylesheet hsl() string.
  //!
  //! If alpha is specified an hsla() form will be returned. The value of
  //! alpha must it must be between 0 (transparent)
  //! and 100 (opaque), the default, otherwise it will be ignored.
  //! If no color has been selected or  'Cancel' has been pressed then
  //! an empty string is returned.
  QString hsl(ColorType type, int alpha = 100) const;

  //! Returns the selected color as a stylesheet name string.
  //!
  //! If no color has been selected then the color will be tested
  //! against all possible X11 name strings and if one matches this
  //! name will be returned.
  //! If 'Cancel' has been pressed then an empty string is returned.
  QString name(ColorType type) const;

  //! Returns the selected color as a #ffffff style string.
  //!
  //! If alpha is specified a #ffffffff form will be returned. The value of
  //! alpha must it must be between 0 (transparent)
  //! and 100 (opaque), the default, otherwise it will be ignored.
  //! If no color has been selected or  'Cancel' has been pressed then
  //! an empty string is returned.
  QString hash(ColorType type, int alpha = false) const;

//  //! Returns the Svg or X11 name for the color if one exists.
//  //! The Svg name is returned in preference to the X11 name if it exists.
//  static QString svgOrX11Name(const QColor& color);
//  //! Returns the Svg or X11 color for the name string
//  //! the name is tested against the Svg colors first.
//  static QColor svgOrX11Color(const QString& initialColor);

//  // This is based on the alternative HSP color model suggested by Darel Rex
//  // Finley. http://alienryderflex.com/hsp.html public domain by Darel
//  // Rex Finley, 2006
//  //! Returns true if the color is a darker shade, otherwise return false.
//  //!
//  //! This can be used to select a text color that shows up well with the
//  //! specified color.
//  static bool isDark(const QColor& color);

signals:
  //! This signal is emitted whenever the primary color changes in the dialog.
  //! The current color is specified by color.
  void primaryColorChanged(const QColor& color, const QString& name);
  void primaryTextColorChanged(const QColor& color, const QString& name);
  //! This signal is emitted whenever the secondary color changes in the dialog.
  //! The current color is specified by color.
  void secondaryColorChanged(const QColor& color, const QString& name);
  void secondaryTextColorChanged(const QColor& color, const QString& name);

protected:
  QSize sizeHint() const override;

private:
  QTabWidget* m_tabs;
  QColor m_primaryColor;
  QColor m_primaryTextColor;
  QColor m_secondaryColor;
  QColor m_secondaryTextColor;
  QString m_primaryName;
  QString m_primaryTextName;
  QString m_secondaryTextName;
  QString m_secondaryName;
  QColorDialog* m_colorDlg;
  ColorDropDisplay* m_display;
  //  Tabs m_currentTab;

  //  void acceptColor();
  void acceptChanges();
  void initGui();
  ColorDragTable* initSvgBlueGreen();
  ColorDragTable* initSvgGreen();
  ColorDragTable* initSvgRedYellowBrown();
  ColorDragTable* initX11BlueGreen();
  ColorDragTable* initX11Red();
  ColorDragTable* initX11YellowBrown();
  ColorDragTable* initX11MonoFrame();
  QDialogButtonBox* createBtnBox();
  ColorDropDisplay* createColorDisplay();
  ColorDragTable* createColorTable(int columns = 7);
  void customMenuRequested(QPoint pos);
  void primaryColorHasChanged(const QColor& color, const QString& name);
  void dialogColorHasChanged(const QColor& color);
  void secondaryColorHasChanged(const QColor& color, const QString& name);
  void primaryTextColorHasChanged(const QColor& color, const QString& name);
  void secondaryTextColorHasChanged(const QColor& color, const QString& name);

  void colorClicked(const QModelIndex& index);
  void primaryColorMenuClicked();
  void primaryTextColorMenuClicked();
  void secondaryColorMenuClicked();
  void secondaryTextColorMenuClicked();

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
