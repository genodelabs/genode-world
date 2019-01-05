
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "highlighter.h"

#include <QMainWindow>
#include <QTextStream>

QT_BEGIN_NAMESPACE
class QTextEdit;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);

public slots:
	void submit();

private:
	void setupEditor(QTextEdit *editor);
	void load();

	QTextEdit *remote_editor;
	QTextEdit *local_editor;
	//Highlighter *highlighter;
};

#endif // MAINWINDOW_H
