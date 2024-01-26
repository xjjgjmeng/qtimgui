#pragma once

#include <QWidget>
#include <QPushButton>

#include "iavTMJViewer.h"

class TMJTab : public QWidget
{
    Q_OBJECT

public:
    TMJTab(QWidget *parent = nullptr);
    ~TMJTab();

private:
    iavTMJViewer* tmjTab;
};