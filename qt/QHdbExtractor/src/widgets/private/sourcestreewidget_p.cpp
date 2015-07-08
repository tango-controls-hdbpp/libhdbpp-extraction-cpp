#include "sourcestreewidget_p.h"
#include "sourcestreewidget.h"

SourcesTreeWidgetPrivate::SourcesTreeWidgetPrivate(SourcesTreeWidget *parent)
    : q_ptr(parent)
{
    thdb = NULL;
}
