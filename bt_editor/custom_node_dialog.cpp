#include "custom_node_dialog.h"
#include "ui_custom_node_dialog.h"
#include "models/ActionNodeModel.hpp"
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QRegExpValidator>

CustomNodeDialog::CustomNodeDialog(const TreeNodeModels &models, QWidget *parent):
    QDialog(parent),
    ui(new Ui::CustomNodeDialog),
    _models(models)
{
    setWindowTitle("Custom TreeNode Editor");
    ui->setupUi(this);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    connect( ui->tableWidget, &QTableWidget::cellChanged,
             this, &CustomNodeDialog::checkValid );

    connect( ui->lineEdit, &QLineEdit::textChanged,
             this, &CustomNodeDialog::checkValid );

    QRegExp rx("\\w+");
    _validator = new QRegExpValidator(rx, this);

    checkValid();
}

CustomNodeDialog::~CustomNodeDialog()
{
    delete ui;
}

std::pair<QString,TreeNodeModel> CustomNodeDialog::getTreeNodeModel() const
{
    TreeNodeModel model;
    switch( ui->comboBox->currentIndex() )
    {
    case 0: model.node_type = NodeType::ACTION; break;
    case 1: model.node_type = NodeType::CONDITION; break;
    }
    for (int row=0; row < ui->tableWidget->rowCount(); row++ )
    {
        const auto key   = ui->tableWidget->item(row,0)->text();
        const auto value = ui->tableWidget->item(row,1)->text();
        model.params.push_back( {key,value} );
    }

    return { ui->lineEdit->text(), model };
}

void CustomNodeDialog::on_toolButtonAdd_pressed()
{
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(row+1);

    ui->tableWidget->setItem(row,0, new QTableWidgetItem( "key_name" ));
    ui->tableWidget->setItem(row,1, new QTableWidgetItem( "default"));

    checkValid();
}

void CustomNodeDialog::on_toolButtonRemove_pressed()
{
    auto selected = ui->tableWidget->selectedItems();
    if(selected.size() == 1)
    {
        int row = selected.first()->row();
        ui->tableWidget->removeRow(row);
    }
    checkValid();
}


void CustomNodeDialog::checkValid()
{
    bool valid = false;
    auto name = ui->lineEdit->text();
    int pos;


    if( name.isEmpty() )
    {
        ui->labelWarning->setText("The name cannot be empty");
    }
    else if( _validator->validate(name, pos) != QValidator::Acceptable)
    {
        ui->labelWarning->setText("Invalid name: use only letters, digits and underscores");
    }
    else if( _models.count( name ) > 0 )
    {
        ui->labelWarning->setText("Another Node has the same name");
    }
    else {

        bool empty_param_name = false;
        bool invalid_param_name = false;
        std::set<QString> param_names;
        for (int row=0; row < ui->tableWidget->rowCount(); row++ )
        {
            auto param_name = ui->tableWidget->item(row,0)->text();
            if(param_name.isEmpty())
            {
                empty_param_name = true;
            }
            else if( _validator->validate(param_name, pos) != QValidator::Acceptable)
            {
                invalid_param_name = true;
            }
            else{
                param_names.insert(param_name);
            }
        }
        if( empty_param_name )
        {
           ui->labelWarning->setText("Empty NodeParameter key");
        }
        else if( invalid_param_name )
        {
            ui->labelWarning->setText("Invalid key: use only letters, digits and underscores");
        }
        else if( param_names.size() < ui->tableWidget->rowCount() ){
           ui->labelWarning->setText("Duplicated NodeParameter key");
        }

        if( param_names.size() == ui->tableWidget->rowCount() )
        {
            valid = true;
        }
    }
    if(valid)
    {
        ui->labelWarning->setText("OK");
        ui->labelWarning->setStyleSheet("color: rgb(78, 154, 6)");
    }
    else{
        ui->labelWarning->setStyleSheet("color: rgb(204, 0, 0)");
    }
    ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( valid );
}