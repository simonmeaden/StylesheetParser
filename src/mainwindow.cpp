/*
   Copyright 2020 Simon Meaden

   Permission is hereby granted, free of charge, to any person obtaining a copy of this
   software and associated documentation files (the "Software"), to deal in the Software
   without restriction, including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software, and to permit
                                                                         persons to whom the Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all copies or
   substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
   INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
    PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
   OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
      SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "stylesheetparser/mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
{
  setCentralWidget(initGui());
  setGeometry(0, 0, 500, 800);
}

MainWindow::~MainWindow() {}

QWidget* MainWindow::initGui()
{
  QFrame* f = new QFrame(this);
  QGridLayout* layout = new QGridLayout;
  f->setLayout(layout);

  QString text;
  //  text =   "color: red";
  //  text =   "color: red;";
  //  text =  "color: red; background: green";
  //  text =  "color: red; background: green;";
  //  text =  "color: red; border: green solid 1px;";
    text =  "color: red border: green solid 1px;"; // missing first end property char.
//    text =  "color: red border: green solid 1px;\n background-color: blue;"; // missing first end property char.
  //  text = "QTabWidget::branch {}";
  //  text = "QTabWidget::branch {color: red}";
  //  text = "QTabWidget::branch {color: red; background: green}";
  //  text = "QTabWidget::branch {color: red; border: green solid 1px; background-color: blue;";
  //  text = "QTabWidget::branch {color: red; border: green solid 1px; background-color: blue;}";
//  text = "QTabWidget::branch {\ncolor: red;\n}\n"
//         "QWidget:active {\nborder: blue;}";
//  text = "QTabWidget:active {\ncolor: red;\n border: green solid 1px;\n background-color: blue;\n}\n"
//         "QWidget::branch {\ncolor: blue;\n background-color: red;\n}";


  m_editor = new StylesheetEdit(this);
  m_editor->setPlainText(text);
  m_editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(m_editor, 0, 0);

  return f;
}
