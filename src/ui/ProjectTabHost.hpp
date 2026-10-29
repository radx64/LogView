#pragma once

#include <QString>

class QWidget;
class FileViewer;

FileViewer* activeFileViewerForTab(QWidget* tab_widget);
QString primaryPathForTab(QWidget* tab_widget);
