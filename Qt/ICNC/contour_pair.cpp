#include "contour_pair.h"

using namespace std;

// Reader
ContourPair::Reader::Reader(const ContourPair& pair) : valid({false, false}), m_pair(pair) {
    if (m_pair.m_bot && !m_pair.m_bot->empty()) {
        it_bot = m_pair.m_bot->entities().begin();
        valid.bot = true;
    }

    if (m_pair.m_top && !m_pair.m_top->empty()) {
        it_top = m_pair.m_top->entities().begin();
        valid.top = true;
    }
}

const DxfEntity* ContourPair::Reader::bot() {
    return (valid.bot && m_pair.m_bot && !m_pair.m_bot->empty() && it_bot != m_pair.m_bot->entities().end()) ? *it_bot : nullptr;
}

const DxfEntity* ContourPair::Reader::top() {
    return (valid.top && m_pair.m_top && !m_pair.m_top->empty() && it_top != m_pair.m_bot->entities().end()) ? *it_top : nullptr;
}

void ContourPair::Reader::next(const DxfEntity*& bot, const DxfEntity*& top) {
    if (valid.bot && m_pair.m_bot && !m_pair.m_bot->entities().empty() && it_bot != m_pair.m_bot->entities().end()) {
        ++it_bot;
        bot = it_bot != m_pair.m_bot->entities().end() ? *it_bot : nullptr;
    }
    else
        bot = nullptr;

    if (valid.top && m_pair.m_top && !m_pair.m_top->entities().empty() && it_top != m_pair.m_top->entities().end()) {
        ++it_top;
        top = it_top != m_pair.m_top->entities().end() ? *it_top : nullptr;
    }
    else
        top = nullptr;
}

// ContourPair
ContourPair::Reader ContourPair::reader(const DxfEntity*& bot, const DxfEntity*& top) const {
    Reader reader(*this);

    bot = reader.bot();
    top = reader.top();

    return reader;
}

GeneratorMode ContourPair::m_genModeDefault = GeneratorMode();
cut_t ContourPair::m_cutDefault = cut_t();

bool ContourPair::emptyBot() const { return !m_bot || m_bot->empty(); }
bool ContourPair::emptyTop() const { return !m_top || m_top->empty(); }
bool ContourPair::empty() const { return emptyBot() && emptyTop(); }
bool ContourPair::isLoop() const { return (!m_bot || m_bot->empty() || m_bot->isLoop()) && (!m_top || m_top->empty() || m_top->isLoop()); }
bool ContourPair::hasOut() const { return m_bot && m_bot->hasOut() && m_top && m_top->hasOut(); }

ContourPair::ContourPair(CONTOUR_TYPE type) :
    m_bot(new Dxf), m_top(new Dxf), m_type(type), m_cut(nullptr), m_genModes(nullptr) {}

ContourPair::ContourPair(const ContourPair& other) :
    m_bot(new Dxf(*other.m_bot)), m_top(new Dxf(*other.m_top)), m_type(other.m_type), m_cut(nullptr), m_genModes(nullptr)
{
    if (other.m_cut) {
        m_cut = new cut_t;
        *m_cut = *other.m_cut;
    }
    if (other.m_genModes && !other.m_genModes->empty()) {
        m_genModes = new deque<GeneratorMode>;
        *m_genModes = *other.m_genModes;
    }
}

ContourPair::ContourPair(CONTOUR_TYPE type, const ContourPair& other) :
    m_bot(new Dxf(*other.m_bot)), m_top(new Dxf(*other.m_top)), m_type(type), m_cut(nullptr), m_genModes(nullptr)
{
    if (other.m_cut) {
        m_cut = new cut_t;
        *m_cut = *other.m_cut;
    }
    if (other.m_genModes && !other.m_genModes->empty()) {
        m_genModes = new deque<GeneratorMode>;
        *m_genModes = *other.m_genModes;
    }
}

ContourPair::ContourPair(CONTOUR_TYPE type, const cut_t* const cut, const std::deque<GeneratorMode>* const modes) :
    m_bot(new Dxf), m_top(new Dxf), m_type(type), m_cut(nullptr), m_genModes(nullptr)
{
    if (cut) {
        m_cut = new cut_t;
        *m_cut = *cut;
    }
    if (modes && !modes->empty()) {
        m_genModes = new deque<GeneratorMode>;
        *m_genModes = *modes;
    }
}

ContourPair::ContourPair(ContourPair&& other) : m_type(other.m_type) {
    m_bot = other.m_bot;
    m_top = other.m_top;
    m_cut = other.m_cut;
    m_genModes = other.m_genModes;
    other.m_bot = new Dxf;
    other.m_top = new Dxf;
    other.m_cut = nullptr;
    other.m_genModes = nullptr;
}

ContourPair::~ContourPair() {
    if (m_bot) {
        delete m_bot;
        m_bot = nullptr;
    }
    if(m_top) {
        delete m_top;
        m_top = nullptr;
    }
    if (m_cut) {
        delete m_cut;
        m_cut = nullptr;
    }
    if (m_genModes) {
        delete m_genModes;
        m_genModes = nullptr;
    }
}

ContourPair& ContourPair::operator=(const ContourPair& other) {
    if (this != &other) {
        clear();
        if (m_bot) {
            delete m_bot;
            m_bot = nullptr;
        }
        if (m_top) {
            delete m_top;
            m_top = nullptr;
        }

        *m_bot = *other.m_bot;
        *m_top = *other.m_top;

        if (other.m_cut) {
            m_cut = new cut_t;
            *m_cut = *other.m_cut;
        }
        if (other.m_genModes && !other.m_genModes->empty()) {
            m_genModes = new deque<GeneratorMode>;
            *m_genModes = *other.m_genModes;
        }
    }

    return *this;
}

ContourPair& ContourPair::operator=(ContourPair&& other) noexcept {
    if (this != &other) {
        clear();
        if(m_bot) {
            delete m_bot;
            m_bot = nullptr;
        }
        if (m_top) {
            delete m_top;
            m_top = nullptr;
        }

        m_bot = other.m_bot;
        m_top = other.m_top;
        m_type = other.m_type;
        m_cut = other.m_cut;
        m_genModes = other.m_genModes;

        other.m_bot = new Dxf;
        other.m_top = new Dxf;
        m_type = CONTOUR_TYPE::CONTOUR_MAIN;
        other.m_cut = nullptr;
        other.m_genModes = nullptr;
    }
    return *this;
}

void ContourPair::clear() {
    if (m_bot) m_bot->clear();
    if (m_top) m_top->clear();

    m_type = CONTOUR_TYPE::CONTOUR_MAIN;

    clearCut();
    clearModes();
}

void ContourPair::setBot(const Dxf& ctr) {
    if (m_bot) delete m_bot;
    m_bot = new Dxf(ctr);
}

Dxf* ContourPair::bot() const { return m_bot; }

void ContourPair::setTop(const Dxf& ctr) {
    if (m_top) delete m_top;
    m_top = new Dxf(ctr);
}

Dxf* ContourPair::top() const { return m_top; }

void ContourPair::setCut(const cut_t& cut) {
    if (!m_cut) m_cut = new cut_t;
    *m_cut = cut;
}

void ContourPair::clearCut() {
    if (m_cut) {
        delete m_cut;
        m_cut = nullptr;
    }
}

const cut_t& ContourPair::cut() const { return m_cut ? *m_cut : m_cutDefault; }

void ContourPair::setGeneratorModes(const std::deque<GeneratorMode>& modes) {
    if (!m_genModes)
        m_genModes = new std::deque<GeneratorMode>;
    else
        m_genModes->clear();

    *m_genModes = modes;
}

void ContourPair::setGeneratorModes(const ContourPair& contour) {
    if (!m_genModes)
        m_genModes = new std::deque<GeneratorMode>;
    else
        m_genModes->clear();

    *m_genModes = *contour.m_genModes;
}

void ContourPair::clearModes() {
    if (m_genModes) {
        delete m_genModes;
        m_genModes = nullptr;
    }
}

const GeneratorMode& ContourPair::generatorMode() const { return m_genModes && !m_genModes->empty() ? m_genModes->front() : m_genModeDefault; }

void ContourPair::choiceMode(uint8_t mode_id) {
    clearCut();
    if (m_genModes) {
        size_t i = 0;
        for (auto it = m_genModes->begin(); it != m_genModes->end(); i++) {
            if (i != mode_id)
                it = m_genModes->erase(it);
            else
                ++it;
        }
    }
}

bool ContourPair::isCutline() const { return m_type == CONTOUR_TYPE::CONTOUR_CUTLINE; }
void ContourPair::setCutline() { m_type = CONTOUR_TYPE::CONTOUR_CUTLINE; }

size_t ContourPair::countBot() const { return m_bot ? m_bot->count() : 0; }
size_t ContourPair::countTop() const { return m_top ? m_top->count() : 0; }
size_t ContourPair::count() const { return std::max(countBot(), countTop()); }

string ContourPair::toStringShort() const {
    return typeToString() + " Bot " + std::to_string(countBot()) + " Top " + std::to_string(countTop());
}

string ContourPair::toString() const {
    string s = "Type: " + typeToString() + "\n";
    s += "Bot, count " + to_string(countBot());
    if (countBot()) {
        s += ":\n";
        s += m_bot->toString();
    }
    else
        s += "\n";

    s += "Top, count " + to_string(countTop());
    if (countTop()) {
        s += ":\n";
        s += m_top->toString();
    }
    else
        s += "\n";

    return s;
}

string ContourPair::typeToString() const {
    switch (m_type) {
        case CONTOUR_TYPE::CONTOUR_MAIN:    return "Contour";
        case CONTOUR_TYPE::CONTOUR_CUTLINE: return "Cutline";
        case CONTOUR_TYPE::CONTOUR_PASS:    return "Pass";
        case CONTOUR_TYPE::CONTOUR_TAB:     return "Tab";
        case CONTOUR_TYPE::CONTOUR_OVERLAP: return "Overlap";
        case CONTOUR_TYPE::CONTOUR_OUT:     return "Out";
    }
    return "Unknown (" + to_string(int(m_type)) + ")";
}

void ContourPair::reverse() {
    if (m_bot) m_bot->reverse();
    if (m_top) m_top->reverse();
}

void ContourPair::setFirst(size_t layer, size_t index) {
    switch (layer) {
    case 0: if (m_bot) m_bot->setFirst(index); break;
    case 1: if (m_top) m_top->setFirst(index); break;
    }
}

void ContourPair::lengthRatio(double len, double& bot_len, double& top_len) const {
    double B, T;

    if (len < M_PRECISION) {
        bot_len = 0;
        top_len = 0;
        return;
    }

    if (m_bot) {
        B = m_bot->length();
        if (B < M_PRECISION) B = 0;
    }
    else
        B = 0;

    if (m_top) {
        T = m_top->length();
        if (T < M_PRECISION) T = 0;
    }
    else
        T = 0;

    if (B < M_PRECISION && T < M_PRECISION) {
        bot_len = 0;
        top_len = 0;
    }
    else if (T < M_PRECISION) {
        bot_len = len < B ? len : B;
        top_len = 0;
    }
    else if (B < M_PRECISION) {
        bot_len = 0;
        top_len = len < T ? len : T;
    }
    else if (B >= T) {
        len = len < T ? len : T;
        bot_len = B / T * len;
        top_len = len;
    }
    else {
        len = len < B ? len : B;
        bot_len = len;
        top_len = T / B * len;
    }
}

void ContourPair::split(double len, size_t& bot_idx, size_t& top_idx) {
    double bot_len, top_len;
    lengthRatio(len, bot_len, top_len);

    bot_idx = m_bot ? m_bot->split(bot_len) : 0;
    top_idx = m_top ? m_top->split(top_len) : 0;
}

void ContourPair::split_rev(double tab, size_t& bot_idx, size_t& top_idx) {
    double bot_tab, top_tab;
    lengthRatio(tab, bot_tab, top_tab);

    bot_idx = m_bot ? m_bot->split_rev(bot_tab) : 0;
    top_idx = m_top ? m_top->split_rev(top_tab) : 0;
}

void ContourPair::remove_before(size_t bot_idx, size_t top_idx) {
    if (m_bot) m_bot->remove_before(bot_idx);
    if (m_top) m_top->remove_before(top_idx);
}

void ContourPair::remove_to(size_t bot_idx, size_t top_idx) {
    if (m_bot) m_bot->remove_to(bot_idx);
    if (m_top) m_top->remove_to(top_idx);
}

void ContourPair::remove_from(size_t bot_idx, size_t top_idx) {
    if (m_bot) m_bot->remove_from(bot_idx);
    if (m_top) m_top->remove_from(top_idx);
}

void ContourPair::remove_after(size_t bot_idx, size_t top_idx) {
    if (m_bot) m_bot->remove_after(bot_idx);
    if (m_top) m_top->remove_after(top_idx);
}

void ContourPair::offset(OFFSET_SIDE side, double offset) {
    if (m_type == CONTOUR_TYPE::CONTOUR_PASS || m_type == CONTOUR_TYPE::CONTOUR_TAB || m_type == CONTOUR_TYPE::CONTOUR_OVERLAP || m_type == CONTOUR_TYPE::CONTOUR_OUT) {
        if (m_bot) m_bot->offset(side, offset);
        if (m_top) m_top->offset(side, offset);
    }
}

fpoint_valid_t ContourPair::firstBot() const { return m_bot ? m_bot->first() : fpoint_valid_t(false); }
fpoint_valid_t ContourPair::firstTop() const { return m_top ? m_top->first() : fpoint_valid_t(false); }
fpoint_valid_t ContourPair::lastBot() const { return m_bot ? m_bot->last() : fpoint_valid_t(false); }
fpoint_valid_t ContourPair::lastTop() const { return m_top ? m_top->last() : fpoint_valid_t(false); }

void ContourPair::changeFirstPoint(const fpoint_valid_t& pt_bot, const fpoint_valid_t& pt_top) {
    m_bot->changeFirstPoint(pt_bot);
    m_top->changeFirstPoint(pt_top);
}

void ContourPair::changeLastPoint(const fpoint_valid_t& pt_bot, const fpoint_valid_t& pt_top) {
    m_bot->changeLastPoint(pt_bot);
    m_top->changeLastPoint(pt_top);
}

void ContourPair::shift(const fpoint_t& pt) {
    if (m_bot) m_bot->shift(pt);
    if (m_top) m_top->shift(pt);
}

vector<fpoint_t> ContourPair::getPointsBot() const {
    return m_bot ? m_bot->getPoints() : vector<fpoint_t>();
}

std::vector<fpoint_t> ContourPair::getPointsTop() const {
    return m_top ? m_top->getPoints() : vector<fpoint_t>();
}

double ContourPair::lengthBot() const {
    return m_bot ? m_bot->length() : 0;
}

double ContourPair::lengthBot(size_t index, const fpoint_t &pt) const {
    return m_bot ? m_bot->length(index, pt) : 0;
}

void ContourPair::rotate(const RotateMatrix& mx) {
    if (m_bot) m_bot->rotate(mx);
    if (m_top) m_top->rotate(mx);
}

void ContourPair::flipX(double x) {
    if (m_bot) m_bot->flipX(x);
    if (m_top) m_top->flipX(x);
}

void ContourPair::flipY(double y) {
    if (m_bot) m_bot->flipY(y);
    if (m_top) m_top->flipY(y);
}

void ContourPair::scale(double k, const fpoint_t& base) {
    if (m_bot) {
        m_bot->shift(-base);
        m_bot->scale(k);
        m_bot->shift(base);
    }
    if (m_top) {
        m_top->shift(-base);
        m_top->scale(k);
        m_top->shift(base);
    }
}

ContourRange ContourPair::range() const {
    ContourRange range, top_range;

    range = Dxf::contourRange(bot()->entities());

    top_range = Dxf::contourRange(top()->entities());
    range.scale(top_range);

    return range;
}
