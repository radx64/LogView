#pragma once

#include <QAbstractItemModel>
#include <QStringList>

class ZipEntryTreeModel : public QAbstractItemModel
{
Q_OBJECT
public:
    explicit ZipEntryTreeModel(QObject* parent = nullptr);
    ~ZipEntryTreeModel() override;

    void setEntryPaths(const QStringList& paths);

    QString entryIdForIndex(const QModelIndex& index) const;
    QModelIndex indexForEntryId(const QString& entry_id) const;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    struct Node
    {
        QString label;
        QString entry_id;
        Node* parent_node = nullptr;
        QVector<Node*> children;
        ~Node();
    };

    Node* nodeForIndex(const QModelIndex& index) const;
    QModelIndex indexForNode(Node* node, int column = 0) const;
    void rebuildTree(const QStringList& paths);
    void clearTree();

    Node* root_ = nullptr;
};
