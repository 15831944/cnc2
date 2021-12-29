#include "mode_table_model.h"
#include <QRegularExpression>

using namespace std;

//ModeTableModel::ModeTableModel(QObject *parent) : QAbstractTableModel(parent) {}
ModeTableModel::ModeTableModel(std::deque<GeneratorMode>& modes, QObject *parent) : QAbstractTableModel(parent), m_modes(modes) {}

QVariant ModeTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    const QRegularExpression re("\\s");

    switch (orientation) {
    case Qt::Horizontal:
        if (role == Qt::DisplayRole)
            switch (section) {
            case 0: return QString( tr("Drum Velocity").replace(re, "\n") );
            case 1: return QString( tr("Pulse Width").replace(re, "\n") );
            case 2: return QString( tr("Pulse Ratio").replace(re, "\n") );
            case 3: return QString( tr("Voltage Level").replace(re, "\n") );
            case 4: return QString( tr("Current Level").replace(re, "\n") );
            }
//        else if (role == Qt::BackgroundRole)
//            return QBrush(Qt::gray);
        break;
    case Qt::Vertical:
        if (role == Qt::DisplayRole)
            return tr("Mode") + QString(" %1").arg(section + 1);
//        else if (role == Qt::BackgroundRole)
//            return QBrush(Qt::gray);
        break;
    }
    return QVariant();
}

int ModeTableModel::rowCount(const QModelIndex& /*parent*/) const { return static_cast<int>(m_modes.size()); }
int ModeTableModel::columnCount(const QModelIndex& /*parent*/) const { return 5; }

QVariant ModeTableModel::data(const QModelIndex &index, int role) const {
    if (index.isValid() && index.row() >= 0 && index.column() >= 0 && size_t(index.row()) < m_modes.size() && index.column() < columnCount()) {
        size_t row = size_t(index.row());
        size_t col = size_t(index.column());

        if (role == Qt::DisplayRole)
            switch (col) {
            case 0: return m_modes[row].drum_velocity;
            case 1: return m_modes[row].pulse_width;
            case 2: return m_modes[row].pulse_ratio;
            case 3: return m_modes[row].voltage_level;
            case 4: return m_modes[row].current_level;
            }
    }

    return QVariant();
}

bool ModeTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (data(index, role) != value && role == Qt::EditRole) {
        size_t row = size_t(index.row());
        size_t col = size_t(index.column());
        bool OK;
        uint32_t data = value.toUInt(&OK);

        if (OK)
            switch (col) {
            case 0: m_modes[row].drum_velocity = data; break;
            case 1: m_modes[row].pulse_width = data; break;
            case 2: m_modes[row].pulse_ratio = data; break;
            case 3: m_modes[row].voltage_level = data; break;
            case 4: m_modes[row].current_level = data; break;
            }

//        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags ModeTableModel::flags(const QModelIndex &index) const {
    if (index.isValid())
        return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled;
    else
        return Qt::NoItemFlags;
}

bool ModeTableModel::insertRows(int row, int count, const QModelIndex &parent) {
    beginInsertRows(parent, row, row + count - 1);

    if (row == int(m_modes.size())) {
        for (int i = 0; i < count; i++)
            m_modes.push_back(GeneratorMode());
    }
    else {
        int i = 0;

        for (auto it = m_modes.begin(); it != m_modes.end(); i++) {
            if (i >= row && i < row + count)
                it = m_modes.insert(it, GeneratorMode());
            else
                ++it;
        }
    }

    endInsertRows();
    return true;
}

bool ModeTableModel::removeRows(int row, int count, const QModelIndex &parent) {
    beginRemoveRows(parent, row, row + count - 1);
    int i = 0;

    for (auto it = m_modes.begin(); it != m_modes.end(); i++) {
        if (i >= row && i < row + count)
            it = m_modes.erase(it);
        else
            ++it;
    }

    endRemoveRows();
    return true;
}
