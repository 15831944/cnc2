#include "contour_list.h"

using namespace std;

ContourList::ContourList() :
    m_contours(deque<ContourPair>()), m_selectedBot(nullptr), m_selectedTop(nullptr),
    m_thickness(0), m_contour(-1), m_segment(-1), m_botValid(false), m_topValid(false), m_botLength(-1), m_error("")
{
}
ContourList::ContourList(const ContourList& other) {
    *this = other;
}
ContourList::ContourList(const deque<ContourPair>& contours) :
    m_contours(contours), m_selectedBot(nullptr), m_selectedTop(nullptr),
    m_thickness(0), m_contour(-1), m_segment(-1), m_botValid(false), m_topValid(false), m_botLength(-1), m_error("")
{
}
ContourList::ContourList(ContourList&& other) {
    *this = move(other);
}
ContourList::ContourList(std::deque<ContourPair>&& contours) :
    m_contours(move(contours)), m_selectedBot(nullptr), m_selectedTop(nullptr),
    m_thickness(0), m_contour(-1), m_segment(-1), m_botValid(false), m_topValid(false), m_botLength(-1), m_error("")
{
}

ContourList::~ContourList() { clear(); }

void ContourList::clear() {
    for (ContourPair& pair: m_contours)
        pair.clear();

    m_contours.clear();
    m_thickness = 0;

    clearSelected();
    clearPos();
    m_botLength = -1;
    m_error.clear();    
}

ContourList& ContourList::operator=(const ContourList& other) {
    if (this != &other) {
        clear();
        m_thickness = other.m_thickness;

        m_contour = other.m_contour;
        m_segment = other.m_segment;

        m_botPos = other.m_botPos;
        m_botValid = other.m_botValid;
        m_topPos = other.m_topPos;
        m_topValid = other.m_topValid;

        for (const ContourPair& pair: other.m_contours)
            m_contours.push_back(pair);

        m_botLength = -1;
    }
    return *this;
}
ContourList &ContourList::operator=(ContourList&& other) noexcept {
    if (this != &other) {
        clear();
        m_contours = move(other.m_contours);
        m_selectedBot = other.m_selectedBot;
        m_selectedTop = other.m_selectedTop;
        m_thickness = other.m_thickness;

        m_contour = other.m_contour;
        m_segment = other.m_segment;

        m_botPos = other.m_botPos;
        m_botValid = other.m_botValid;
        m_topPos = other.m_topPos;
        m_topValid = other.m_topValid;
        m_botLength = -1;

        m_error = move(other.m_error);
    }
    return *this;
}

void ContourList::replace(deque<ContourPair>& contours) {
    if (&m_contours != &contours) {
        clear();
        m_contours = contours;
        contours.clear();
    }
}

//void ContourList::setCut(const cut_t& cut) {
//    for (ContourPair& pair: m_contours)
//        pair.setCut(cut);
//}

void ContourList::setCut(const cut_t& cut, const deque<GeneratorMode>& modes) {
    for (ContourPair& pair: m_contours) {
        pair.setCut(cut);
        pair.setGeneratorModes(modes);

        if (pair.type() == CONTOUR_TYPE::CONTOUR_CUTLINE) {
            pair.choiceMode(pair.cut().cutline_mode_id);
        }
    }
}

//void ContourList::setGeneratorModes(const std::deque<GeneratorMode>& modes) {
//    for (ContourPair& pair: m_contours)
//        pair.setGeneratorModes(modes);
//}

void ContourList::push_front(const ContourPair &pair) {
    m_contours.push_front(pair);
    m_botLength = -1;
}
void ContourList::push_back(const ContourPair &pair) {
    m_contours.push_back(pair);
    m_botLength = -1;
}
void ContourList::insert(size_t index, const ContourPair& pair) {
    size_t i = 0;

    for (auto it = m_contours.begin(); it != m_contours.end(); ++it, i++) {
        if (i == index) {
            m_contours.insert(it, pair);
            break;
        }
    }
    m_botLength = -1;
}

void ContourList::insert_after(size_t index, const ContourPair& pair) {
    if (index == m_contours.size() - 1)
        m_contours.push_back(pair);
    else {
        size_t i = 0;

        for (auto it = m_contours.begin(); it != m_contours.end(); ++it, i++) {
            if (i == index + 1) {
                m_contours.insert(it, pair);
                break;
            }
        }
    }
    m_botLength = -1;
}

void ContourList::new_insert(size_t index) { insert(index, ContourPair()); }
void ContourList::new_front() { m_contours.push_front(ContourPair()); }
void ContourList::new_back() { m_contours.push_back(ContourPair()); }

bool ContourList::remove(size_t index) {
    size_t i = 0;
    m_selectedBot = nullptr;
    m_selectedTop = nullptr;
    m_botLength = -1;

    for (auto it = m_contours.begin(); it != m_contours.end(); ++it, i++) {
        if (i == index) {
            it->clear();
            m_contours.erase(it);
            return true;
        }
    }
    return false;
}

ContourPair* ContourList::at(size_t index) {
    size_t i = 0;
    m_botLength = -1;

    if (index < m_contours.size())
        for (auto it = m_contours.begin(); it != m_contours.end(); ++it, i++) {
            if (i == index)
                return &(*it);
        }

    return nullptr;
}

const ContourPair* ContourList::at(size_t index) const {
    size_t i = 0;

    for (auto it = m_contours.cbegin(); it != m_contours.cend(); ++it, i++) {
        if (i == index)
            return &(*it);
    }

    return nullptr;
}

ContourPair* ContourList::front() {
    m_botLength = -1;
    return m_contours.empty() ? nullptr : &m_contours.front();
}
ContourPair* ContourList::back() {
    m_botLength = -1;
    return m_contours.empty() ? nullptr : &m_contours.back();
}

bool ContourList::empty() const { return m_contours.empty(); }

size_t ContourList::count() const { return m_contours.size(); }

const std::string &ContourList::lastError() { return m_error; }

ContourRange ContourList::range() const {
    ContourRange range, bot_range, top_range;

    for (const ContourPair& pair: m_contours) {
        bot_range = pair.bot() ? Dxf::contourRange(pair.bot()->entities()) : ContourRange();
        top_range = pair.top() ? Dxf::contourRange(pair.top()->entities()) : ContourRange();

        range.scale(bot_range);
        range.scale(top_range);
    }

    double w = range.width();
    double h  = range.height();

    if (w == 0)
        range.expandWidth(0.2 * h);
    else if (h == 0)
        range.expandHeight(0.2 * w);
    else if (w / h < 0.2)
        range.expandWidth(0.2 * h);
    else if (h / w < 0.2)
        range.expandHeight(0.2 * w);

    return range;
}

void ContourList::select(size_t contour_num, size_t row, size_t column) {
    clearSelected();

    if (contour_num < m_contours.size()) {
        ContourPair* pair = at(contour_num);

        switch (column) {
            case 0:
                if (pair && row < pair->countBot())
                    m_selectedBot = pair->bot()->at(row);
                else
                    m_selectedBot = nullptr;

                return;
            case 1:
                if (pair && row < pair->countTop())
                    m_selectedTop = pair->top()->at(row);
                else
                    m_selectedTop = nullptr;

                return;
        }
    }
}

void ContourList::select(const std::pair<size_t, size_t>& ctr_ent) {
    clearSelected();
    size_t n = ctr_ent.first;
    size_t m = ctr_ent.second;

    if (n < m_contours.size() && n < INT32_MAX && m < m_contours[n].count() && m < INT32_MAX) {
        ContourPair* pair = at(n);

        m_selectedBot = pair && pair->bot() ? pair->bot()->at(m) : nullptr;
        m_selectedTop = pair && pair->top() ? pair->top()->at(m) : nullptr;
        m_contour = int(n);
        m_segment = int(m);
    }
}

void ContourList::clearSelected() {
    m_selectedBot = nullptr;
    m_selectedTop = nullptr;
    m_contour = -1;
    m_segment = -1;
    m_botValid = m_topValid = false;
}

bool ContourList::isSelected() const { return !m_contours.empty() && (m_selectedBot != nullptr || m_selectedTop != nullptr || (m_contour >= 0 && m_segment >= 0)); }

void ContourList::setBotPos(const fpoint_t &pt) {
    m_botPos = pt;
    m_botValid = true;
}
void ContourList::setTopPos(const fpoint_t &pt) {
    m_topPos = pt;
    m_topValid = true;
}
void ContourList::clearPos() { m_botValid = m_topValid = false; }
bool ContourList::isBotPos() const { return m_botValid; }
bool ContourList::isTopPos() const { return m_topValid; }
const fpoint_t *ContourList::botPos() const { return m_botValid ? &m_botPos : nullptr; }
const fpoint_t *ContourList::topPos() const { return m_topValid ? &m_topPos : nullptr; }

const DxfEntity* ContourList::selectedBot() const {
    return m_contours.empty() ? nullptr : m_selectedBot;
}

const DxfEntity* ContourList::selectedTop() const {
    return m_contours.empty() ? nullptr : m_selectedTop;
}

//const DxfEntity* ContourList::botSelected() const {
//    if (m_contour >= 0 && size_t(m_contour) < m_contours.size() && m_segment >= 0 && size_t(m_segment) < m_contours[size_t(m_contour)].countBot()) {
//        return m_contours[size_t(m_contour)].bot().at(size_t(m_segment));
//    }
//    return nullptr;
//}

//const DxfEntity *ContourList::topSelected() const {
//    if (m_contour >= 0 && size_t(m_contour) < m_contours.size() && m_segment >= 0 && size_t(m_segment) < m_contours[size_t(m_contour)].countTop()) {
//        return m_contours[size_t(m_contour)].top().at(size_t(m_segment));
//    }
//    return nullptr;
//}

string ContourList::toString() const {
    std::string s;

    s += "Size " + to_string(m_contours.size()) + "\n";
    int i = 0;
    for (const ContourPair& pair: m_contours) {
        s += to_string(i++) + " " + pair.toStringShort() + "\n";
    }

    return s;
}

void ContourList::shift(deque<ContourPair>& contours, const fpoint_t& pt) {
    for (ContourPair& pair: contours)
        pair.shift(pt);
}

void ContourList::shift(const fpoint_t& pt) { shift(m_contours, pt); }
void ContourList::shift() {
    fpoint_valid_t pt = firstBot();
    if (pt.valid) shift(-pt);
}

fpoint_valid_t ContourList::firstBot() const { return m_contours.empty() ? fpoint_valid_t(false) : m_contours.front().firstBot(); }
fpoint_valid_t ContourList::firstTop() const { return m_contours.empty() ? fpoint_valid_t(false) : m_contours.front().firstTop(); }

bool ContourList::anyTop() const {
    for (const ContourPair& pair: m_contours) {
        if (!pair.emptyTop())
            return true;
    }
    return false;
}

bool ContourList::verify() {
    int i = 0;

    for (auto it = m_contours.cbegin(); ; i++) {
        if (it->emptyBot()) {
            m_error = "Bottom contour " + to_string(i + 1) + " is empty";
            return false;
        }

        auto nx = next(it);

        if (nx != m_contours.cend()) {
            if (it->bot()->front()->point_1() != nx->bot()->front()->point_0()) {
                m_error = "Last point of bottom contour " + to_string(i + 1) + " is not connected to first point of bottom contour " + to_string(i + 2);
                return false;
            }
        }
        else
            break;

        it = nx;
    }

    if (anyTop()) {
        for (auto it = m_contours.cbegin(); ; i++) {
            if (it->emptyTop()) {
                m_error = "Top contour " + to_string(i + 1) + " is empty";
                return false;
            }
            else if (it->countTop() != it->countBot()) {
                m_error = "In contour, " + to_string(i+1) +
                        " bottom entities number " + to_string(it->countBot()) +
                        " is not equal top entities number " + to_string(it->countTop());
                return false;
            }

            auto nx = next(it);

            if (nx != m_contours.cend()) {
                if (it->bot()->front()->point_1() != nx->bot()->front()->point_0()) {
                    m_error = "Last point of top contour " + to_string(i) + " is not connected to first point of top contour " + to_string(i + 1);
                    return false;
                }
            }
            else
                break;

            it = nx;
        }
    }

    return true;
}

void ContourList::setCurrent(int contour_num, int segment_num) {
    if (m_contour >= 0 && size_t(m_contour) < m_contours.size()) {
        m_contour = contour_num;

        if (segment_num >= 0 && size_t(segment_num) < m_contours.at(size_t(m_contour)).count()) {
            m_segment = segment_num;
            return;
        }
    }

    m_contour = -1;
    m_segment = -1;
}

// todo: cacl ones, use botLengthFullForce?
double ContourList::botLengthFull() {
    if (m_botLength <= 0) {
        m_botLength = 0;
        for (const ContourPair& pair: m_contours)
            m_botLength += pair.lengthBot();
    }

    return m_botLength;
}

double ContourList::botLength() const {
    double sum = 0;

    if (m_botValid && m_contour >= 0 && size_t(m_contour) < m_contours.size() && m_segment >= 0 && size_t(m_segment) < m_contours[size_t(m_contour)].count()) {
        if (m_contour > 0) {
            for (size_t i = 0; i < size_t(m_contour) - 1; i++)
                sum += m_contours[i].lengthBot();
        }

        sum += m_contours[size_t(m_contour)].lengthBot(size_t(m_segment), m_botPos);
    }

    return sum;
}

double ContourList::pct() {
    return botLength() / botLengthFull() * 100;
}

void ContourList::rotate(double phi) {
    RotateMatrix mx(phi);

    for (ContourPair& pair: m_contours)
        pair.rotate(mx);
}

void ContourList::flipX() {
    ContourPair* pair = front();
    if (!pair || !pair->bot()) return;

    double x = pair->bot()->first().x;

    for (ContourPair& pair: m_contours)
        pair.flipX(x);
}

void ContourList::flipY() {
    ContourPair* pair = front();
    if (!pair || !pair->bot()) return;

    double y = pair->bot()->first().y;

    for (ContourPair& pair: m_contours)
        pair.flipY(y);
}
