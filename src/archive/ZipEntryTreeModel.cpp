#include "ZipEntryTreeModel.hpp"

#include <algorithm>

ZipEntryTreeModel::Node::~Node()
{
    qDeleteAll(children);
}

ZipEntryTreeModel::ZipEntryTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
    , root_(new Node())
{
    root_->label = QStringLiteral("/");
}

ZipEntryTreeModel::~ZipEntryTreeModel()
{
    delete root_;
    root_ = nullptr;
}

void ZipEntryTreeModel::clearTree()
{
    beginResetModel();
    delete root_;
    root_ = new Node();
    root_->label = QStringLiteral("/");
    endResetModel();
}

void ZipEntryTreeModel::setEntryPaths(const QStringList& paths)
{
    beginResetModel();
    delete root_;
    root_ = new Node();
    root_->label = QStringLiteral("/");

    for (const QString& path : paths)
    {
        const QStringList parts = path.split(QLatin1Char('/'), Qt::SkipEmptyParts);
        Node* current = root_;
        QString built;
        for (int i = 0; i < parts.size(); ++i)
        {
            built += parts[i];
            const bool is_leaf = (i == parts.size() - 1);

            Node* next = nullptr;
            for (Node* child : current->children)
            {
                if (child->label == parts[i])
                {
                    next = child;
                    break;
                }
            }
            if (!next)
            {
                next = new Node();
                next->label = parts[i];
                next->parent_node = current;
                current->children.append(next);
            }
            if (is_leaf)
                next->entry_id = path;
            current = next;
            if (!is_leaf)
                built += QLatin1Char('/');
        }
    }

    auto sort_children = [](auto&& self, Node* node) -> void
    {
        std::sort(node->children.begin(), node->children.end(),
                  [](Node* a, Node* b) { return a->label.compare(b->label, Qt::CaseInsensitive) < 0; });
        for (Node* child : node->children)
            self(self, child);
    };
    sort_children(sort_children, root_);
    endResetModel();
}

ZipEntryTreeModel::Node* ZipEntryTreeModel::nodeForIndex(const QModelIndex& index) const
{
    if (!index.isValid())
        return root_;
    return static_cast<Node*>(index.internalPointer());
}

QModelIndex ZipEntryTreeModel::indexForNode(Node* node, int column) const
{
    if (!node || node == root_)
        return QModelIndex();
    return createIndex(node->parent_node->children.indexOf(node), column, node);
}

QModelIndex ZipEntryTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    Node* parent_node = nodeForIndex(parent);
    if (!parent_node || row < 0 || row >= parent_node->children.size())
        return QModelIndex();
    return createIndex(row, column, parent_node->children.at(row));
}

QModelIndex ZipEntryTreeModel::parent(const QModelIndex& index) const
{
    Node* node = nodeForIndex(index);
    if (!node || !node->parent_node || node->parent_node == root_)
        return QModelIndex();
    return indexForNode(node->parent_node);
}

int ZipEntryTreeModel::rowCount(const QModelIndex& parent) const
{
    Node* parent_node = nodeForIndex(parent);
    return parent_node ? parent_node->children.size() : 0;
}

int ZipEntryTreeModel::columnCount(const QModelIndex& parent) const
{
    (void) parent;
    return 1;
}

QVariant ZipEntryTreeModel::data(const QModelIndex& index, int role) const
{
    Node* node = nodeForIndex(index);
    if (!node || node == root_)
        return QVariant();

    if (role == Qt::DisplayRole)
        return node->label;
    if (role == Qt::ToolTipRole && !node->entry_id.isEmpty())
        return node->entry_id;
    return QVariant();
}

QString ZipEntryTreeModel::entryIdForIndex(const QModelIndex& index) const
{
    Node* node = nodeForIndex(index);
    return node ? node->entry_id : QString();
}

QModelIndex ZipEntryTreeModel::indexForEntryId(const QString& entry_id) const
{
    if (entry_id.isEmpty())
        return QModelIndex();

    const QStringList parts = entry_id.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    Node* current = root_;
    for (const QString& part : parts)
    {
        Node* next = nullptr;
        for (Node* child : current->children)
        {
            if (child->label == part)
            {
                next = child;
                break;
            }
        }
        if (!next)
            return QModelIndex();
        current = next;
    }
    return indexForNode(current);
}
