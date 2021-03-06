#include "ui_ErrorEditDialog.h"
#include "ErrorEditDialog.h"

#include <QTableWidgetItem>
#include <QStringList>
#include <QDebug>
#include <QBrush>
#include <QColor>
#include <QAbstractButton>
#include <QMessageBox>
#include <sstream>

/*
 * Load data contained in the errors vector into a QWidgetTable
 * Fields will be marked red and editable if they are a mandatory field
 * and editable.  Otherwise all other fields will not be editable.
 * Clicking Save in the dialog will return the corrected entries to the main
 * program through the errors parameter.  If not all marked fields are edited
 * then a warning message will be displayed.  If cancel is clicked all errors
 * are discarded.
 */
ErrorEditDialog::ErrorEditDialog(QWidget *parent,
                                 std::vector<std::vector<std::string>*>& errors,
                                 std::vector<std::string>& headers,
                                 std::vector<std::string>& mandatory) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    errorList(errors),
    headerList(headers),
    mandatoryList(mandatory),
    ui(new Ui::ErrorEditDialog)
{
    ui->setupUi(this);
    ui->tableWidget->setRowCount((int) errors.size());
    ui->tableWidget->setColumnCount((int) headers.size());

    QStringList listHeaders;
    for (int i = 0; i < (int) headers.size(); i++) {
        listHeaders << headers[i].c_str();
    }

    ui->tableWidget->setHorizontalHeaderLabels(listHeaders);
    QTableWidgetItem* item;
    QBrush brush(QColor(255, 0, 0, 100));
    std::vector<std::vector<std::string>*>::iterator it;
    int row = 0, count = 0;
    for (it = errors.begin(); it != errors.end(); it++) {
        for (int col = 0; col < (int) headers.size() && col < (int) (*it)->size(); col++) {
            item = new QTableWidgetItem();
            Qt::ItemFlags flag = item->flags();
            item->setFlags(Qt::ItemIsSelectable);
            item->setText((*it)->at(col).c_str());
            for (int i = 0; i < (int) mandatory.size(); i++) {
                if (mandatory[i].compare(headers.at(col)) == 0
                        && (*it)->at(col).compare("") == 0) {
                    item->setBackground(brush);
                    item->setFlags(flag);
                    count++;
                }
            }
            ui->tableWidget->setItem(row, col, item);
        }
        row++;
    }
    populated = true;
    std::stringstream s;
    s << count << " Remaining Errors";
    ui->errors_label->setText(QString::fromStdString(s.str()));
}

//Clean up allocated memory for the table items
ErrorEditDialog::~ErrorEditDialog()
{
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        for (int j = 0; j < ui->tableWidget->columnCount(); j++) {
            delete ui->tableWidget->item(i,j);
        }
    }
    delete ui;
}

//Save the new data entered by the user via the error reference var
void ErrorEditDialog::saveData() {
    for (int row = 0; row < ui->tableWidget->rowCount(); row++) {
        for (int col = 0; col < ui->tableWidget->columnCount() && col < (int) errorList[row]->size(); col++) {
            std::vector<std::string>::iterator it = errorList[row]->begin()+col;
            if (errorList[row]->at(col).compare("") == 0) {
                it = errorList[row]->erase(it);
                errorList[row]->insert(it, ui->tableWidget->item(row, col)->text().toStdString());
            }
        }
    }
    accept();
}

void ErrorEditDialog::on_save_clicked()
{
    bool search = true, save = true;
    //check if mandatory fields have been filled
    for (int row = 0; row < ui->tableWidget->rowCount() && search; row++) {
        for (int j = 0; j < (int) mandatoryList.size() && search; j++) {
            std::vector<std::string>::iterator it = std::find(headerList.begin(), headerList.end(), mandatoryList[j]);
            int col = it - headerList.begin();
            QTableWidgetItem* item = ui->tableWidget->item(row, col);

            if (item->text().compare("") == 0) {
                //warn of unedited fields, allow saving anyways
                QMessageBox::StandardButton reply;
                reply= QMessageBox::critical(this, "Error", "Mandatory fields are still empty Save anyways?.",QMessageBox::Yes|QMessageBox::No);
                save = reply == QMessageBox::Yes;

                search = false;
            }
        }
    }
    if (save) {
        saveData();
    }
}

void ErrorEditDialog::on_cancel_clicked()
{
    reject();
}


//Next button handling to navigate to next editable entry
void ErrorEditDialog::on_NextBtn_clicked(){
    //if no selected items, treat as if already at selected item
    bool foundSelected = ui->tableWidget->selectedItems().size() == 0;

    //loop, allowing for 2 cycles to handle back to top
    for (int cycle = 0,row = 0; row < ui->tableWidget->rowCount() && cycle < 2; row++) {
        for (int j = 0; j < (int) mandatoryList.size(); j++) {
            std::vector<std::string>::iterator it = std::find(headerList.begin(), headerList.end(), mandatoryList[j]);
            int col = it - headerList.begin();
            QTableWidgetItem* item = ui->tableWidget->item(row, col);

            //if we have found the selected item look for next blank error field
            if (foundSelected && item->text().compare("") == 0) {
                ui->tableWidget->scrollToItem(item);
                ui->tableWidget->clearSelection();
                ui->tableWidget->setCurrentItem(item);
                item->setSelected(true);
                ui->tableWidget->editItem(item);
                return;
            }
            //search for selected item
            if(item->isSelected()){
                foundSelected = true;
            }
        }
        //increase cycle and reset row to wrap to top
        if(row == ui->tableWidget->rowCount() - 1){
            cycle++;
            row = -1;  //set to one below start, will be incremented before use
        }
    }
}

//navigates to previous erroneuos entry
void ErrorEditDialog::on_PrevBtn_clicked(){
    //allow button to work with no selection
    bool foundSelected = ui->tableWidget->selectedItems().size() == 0;

    //loop, allowing for 2 cycles to handle back to top
    for (int cycle = 0,row = ui->tableWidget->rowCount() - 1; row >= 0 && cycle < 2; row--) {
        for (int j = (int) mandatoryList.size() - 1; j >= 0; j--) {
            std::vector<std::string>::iterator it = std::find(headerList.begin(), headerList.end(), mandatoryList[j]);
            int col = it - headerList.begin();
            QTableWidgetItem* item = ui->tableWidget->item(row, col);

            //if we have found the selected item look for next blank error field
            if (foundSelected && item->text().compare("") == 0) {
                ui->tableWidget->scrollToItem(item);
                ui->tableWidget->clearSelection();
                ui->tableWidget->setCurrentItem(item);
                item->setSelected(true);
                ui->tableWidget->editItem(item);
                return;
            }
            //search for selected item
            if(item->isSelected()){
                foundSelected = true;
            }
        }
        //allow second cycle of search to wrap around to bottom
        if(row == 0){
            cycle++;
            row = ui->tableWidget->rowCount();  //set to one above start, will be decremented bedore use
        }
    }
}

//update error count when an item is changed
void ErrorEditDialog::on_tableWidget_itemChanged(QTableWidgetItem * item){
    //traverse table to prevent miscounts
    if(populated){
        int count = 0;
        for (int row = 0; row < ui->tableWidget->rowCount(); row++) {
            for (int j = 0; j < (int) mandatoryList.size(); j++) {
                std::vector<std::string>::iterator it = std::find(headerList.begin(), headerList.end(), mandatoryList[j]);
                int col = it - headerList.begin();
                QTableWidgetItem* item = ui->tableWidget->item(row, col);
                if (item->text().compare("") == 0) {
                    //add to count of error entries
                    count++;
                }
            }
        }
        std::stringstream s;
        s << count << " Remaining Errors";
        ui->errors_label->setText(QString::fromStdString(s.str()));
    }
}

