#pragma once

#include <QWidget>
#include <QPushButton>

#include "iavCPRViewer.h"

class CPRTab : public QWidget
{
    Q_OBJECT

public:
    CPRTab(QWidget *parent = nullptr);
    ~CPRTab();

private:
    QPushButton* m_btn=nullptr;
    QPushButton* m_btnCpr = nullptr;

    iavCPRViewer* cprTab;
};