#include "stdafx.h"
#include "MailWindow.h"

void MailWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<MailWindow>();
}

MailWindow::MailWindow(Context* context) :
    Window(context)
{
    SubscribeToEvents();
    SetName("MailWindow");
}

void MailWindow::SubscribeToEvents()
{
}
