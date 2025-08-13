// main.cpp
//  Study Schedule Generator 

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>
#include <QPlainTextEdit>
#include <QGroupBox>
#include <QScrollArea>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QBrush>
#include <QColor>
#include <QComboBox>

#include <vector>
#include <string>
#include <queue>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <set>

using namespace std;

static constexpr double EPSILON = 0.01;
static constexpr int DEFAULT_DAYS = 14;
static constexpr double DEFAULT_HOURS_PER_DAY = 4.0;
static constexpr int MAX_DAYS = 365;

// Subject model class
class Subject {
private:
    string name;
    int difficulty;
    int importance;
    int topics;
    double remainingHours;
    vector<string> topicsList;
public:
    Subject() : name(""), difficulty(1), importance(1), topics(0), remainingHours(0.0) {}
    Subject(const string &n, int diff, int imp, int t, const vector<string> &topicNames)
        : name(n), difficulty(diff), importance(imp), topics(t), remainingHours(0.0), topicsList(topicNames) {}

    string getName() const { return name; }
    int getDifficulty() const { return difficulty; }
    int getImportance() const { return importance; }
    int getTopicsCount() const { return topics; }
    double getRemainingHours() const { return remainingHours; }
    void setRemainingHours(double hrs) { remainingHours = hrs; }

    bool hasTopics() const { return !topicsList.empty(); }

    string getTopicAtIndex(size_t idx) const {
        if (topicsList.empty()) return string("Topic");
        return topicsList[idx % topicsList.size()];
    }

    void addTopic(const string &t) { topicsList.push_back(t); }
    void setName(const string &n) { name = n; }
    void setDifficulty(int d) { difficulty = d; }
    void setImportance(int i) { importance = i; }
    void setTopics(int t) { topics = t; }
    void setTopicsList(const vector<string> &tlist) { topicsList = tlist; topics = (int)tlist.size(); }
};

// Task and ScheduleGenerator class
class Task {
public:
    string subject;
    string topic;
    double hours;
    Task(const string &s, const string &t, double h) : subject(s), topic(t), hours(h) {}
};

class ScheduleGenerator {
private:
    vector<Subject> subjects;
    vector<vector<Task>> schedule;
    int days;
    double hoursPerDay;
public:
    ScheduleGenerator(int d, double hpd) : days(d), hoursPerDay(hpd) {
        schedule.resize(days);
    }

    void setSubjects(const vector<Subject> &s) { subjects = s; }

    void generateSchedule() {
        vector<double> weights;
        double totalWeight = 0.0;
        for (auto &s : subjects) {
            double w = s.getDifficulty() * s.getImportance() * max(1, s.getTopicsCount());
            weights.push_back(w);
            totalWeight += w;
        }

        double totalAvailableHours = days * hoursPerDay;
        for (int i = 0; i < (int)subjects.size(); ++i) {
            double assignedHours = (totalWeight > 0) ? (weights[i] / totalWeight) * totalAvailableHours : 0;
            subjects[i].setRemainingHours(assignedHours);
        }

        schedule.clear();
        schedule.resize(days);
        vector<size_t> topicIndices(subjects.size(), 0);

        for (int d = 0; d < days; ++d) {
            double left = hoursPerDay;
            bool assignedSomething = true;

            while (left > EPSILON && assignedSomething) {
                assignedSomething = false;

                for (int i = 0; i < (int)subjects.size() && left > EPSILON; ++i) {
                    if (subjects[i].getRemainingHours() <= EPSILON) continue;

                    double toAssign = min(left, subjects[i].getRemainingHours());
                    string topic = subjects[i].getTopicAtIndex(topicIndices[i]);
                    topicIndices[i]++;
                    schedule[d].push_back(Task(subjects[i].getName(), topic, toAssign));
                    subjects[i].setRemainingHours(subjects[i].getRemainingHours() - toAssign);
                    left -= toAssign;
                    assignedSomething = true;
                }
            }
        }
    }

    const vector<vector<Task>>& getSchedule() const { return schedule; }
};

// AddSubjectDialog 

class AddSubjectDialog : public QDialog {
    Q_OBJECT
public:
    AddSubjectDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle("Add Subject");
        QVBoxLayout *main = new QVBoxLayout;
        QFormLayout *form = new QFormLayout;

        nameEdit = new QLineEdit;
        diffSpin = new QSpinBox; diffSpin->setRange(1,10); diffSpin->setValue(5);
        impSpin = new QSpinBox; impSpin->setRange(1,10); impSpin->setValue(5);
        topicsEdit = new QPlainTextEdit;
        topicsEdit->setPlaceholderText("Enter one topic per line");
        topicsEdit->setFixedHeight(120);

        form->addRow("Name:", nameEdit);
        form->addRow("Difficulty (1-10):", diffSpin);
        form->addRow("Importance (1-10):", impSpin);
        form->addRow("Topics (one per line):", topicsEdit);

        main->addLayout(form);

        QHBoxLayout *btns = new QHBoxLayout;
        QPushButton *ok = new QPushButton("OK");
        QPushButton *cancel = new QPushButton("Cancel");
        btns->addStretch(); btns->addWidget(ok); btns->addWidget(cancel);
        main->addLayout(btns);

        setLayout(main);

        connect(ok, &QPushButton::clicked, this, &AddSubjectDialog::onOk);
        connect(cancel, &QPushButton::clicked, this, &AddSubjectDialog::reject);
    }

    string getName() const { return name.toStdString(); }
    int getDifficulty() const { return diff; }
    int getImportance() const { return imp; }
    vector<string> getTopics() const { return topics; }

private slots:
    void onOk() {
        name = nameEdit->text().trimmed();
        if (name.isEmpty()) {
            QMessageBox::warning(this, "Input error", "Subject name cannot be empty.");
            return;
        }
        diff = diffSpin->value();
        imp = impSpin->value();

        QString ttext = topicsEdit->toPlainText().trimmed();
        QStringList lines = ttext.split('\n', Qt::SkipEmptyParts);
        topics.clear();
        for (const QString &l : lines) {
            QString trimmed = l.trimmed();
            if (!trimmed.isEmpty()) topics.push_back(trimmed.toStdString());
        }
        if (topics.empty()) {
            QMessageBox::warning(this, "Input error", "Please enter at least one topic.");
            return;
        }

        accept();
    }

private:
    QLineEdit *nameEdit;
    QSpinBox *diffSpin;
    QSpinBox *impSpin;
    QPlainTextEdit *topicsEdit;

    QString name;
    int diff;
    int imp;
    vector<string> topics;
};

// MainWindow 

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        QWidget *central = new QWidget;
        QVBoxLayout *mainLayout = new QVBoxLayout;

        // Controls
        QGroupBox *controlsBox = new QGroupBox("Schedule Settings");
        QFormLayout *controlsLayout = new QFormLayout;

        daysSpin = new QSpinBox; daysSpin->setRange(1,MAX_DAYS); daysSpin->setValue(DEFAULT_DAYS);
        hoursSpin = new QDoubleSpinBox; hoursSpin->setRange(0.5,24.0); hoursSpin->setSingleStep(0.5); hoursSpin->setValue(DEFAULT_HOURS_PER_DAY);

        controlsLayout->addRow("Days:", daysSpin);
        controlsLayout->addRow("Hours per day:", hoursSpin);
        controlsBox->setLayout(controlsLayout);

        mainLayout->addWidget(controlsBox);

        // Highlight Filter combo box
        QHBoxLayout* filterLayout = new QHBoxLayout;
        QLabel* filterLabel = new QLabel("Highlight filter:");
        filterCombo = new QComboBox;
        filterCombo->addItem("All (combined)");
        filterCombo->addItem("Difficulty only");
        filterCombo->addItem("Topics only");
        filterCombo->addItem("Hours only");
        filterLayout->addWidget(filterLabel);
        filterLayout->addWidget(filterCombo);
        filterLayout->addStretch();
        mainLayout->addLayout(filterLayout);

        // Subject table
        subjectTable = new QTableWidget(0,4);
        subjectTable->setHorizontalHeaderLabels({"Name","Difficulty","Importance","#Topics"});
        subjectTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        subjectTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

        QPushButton *addSubjectBtn = new QPushButton("Add Subject");
        QPushButton *removeSubjectBtn = new QPushButton("Remove Selected");

        QHBoxLayout *subjectBtns = new QHBoxLayout;
        subjectBtns->addWidget(addSubjectBtn);
        subjectBtns->addWidget(removeSubjectBtn);
        subjectBtns->addStretch();

        // Action buttons
        QPushButton *generateBtn = new QPushButton("Generate Schedule");
        QPushButton *saveBtn = new QPushButton("Save CSV");
        QPushButton *clearBtn = new QPushButton("Clear Schedule");

        QHBoxLayout *actionBtns = new QHBoxLayout;
        actionBtns->addWidget(generateBtn);
        actionBtns->addWidget(saveBtn);
        actionBtns->addWidget(clearBtn);
        actionBtns->addStretch();

        // Schedule table
        scheduleTable = new QTableWidget(0,4);
        scheduleTable->setHorizontalHeaderLabels({"Day","Subject","Topic","Time"});
        scheduleTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        // Layout assembly
        mainLayout->addWidget(new QLabel("Subjects"));
        mainLayout->addWidget(subjectTable);
        mainLayout->addLayout(subjectBtns);
        mainLayout->addLayout(actionBtns);
        mainLayout->addWidget(new QLabel("Generated Schedule"));
        mainLayout->addWidget(scheduleTable);

        central->setLayout(mainLayout);
        setCentralWidget(central);
        setWindowTitle("Study Schedule Generator (Improved)");
        resize(900, 720);

        // Connections
        connect(addSubjectBtn, &QPushButton::clicked, this, &MainWindow::onAddSubject);
        connect(removeSubjectBtn, &QPushButton::clicked, this, &MainWindow::onRemoveSubject);
        connect(generateBtn, &QPushButton::clicked, this, &MainWindow::onGenerate);
        connect(saveBtn, &QPushButton::clicked, this, &MainWindow::onSave);
        connect(clearBtn, &QPushButton::clicked, this, &MainWindow::onClearSchedule);
        connect(daysSpin, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::onDaysChanged);
        connect(filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onFilterChanged);

        currentFilter = HighlightFilter::All;
        updateHighlightSpinRange();
    }

private slots:
    void onAddSubject() {
        AddSubjectDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            Subject s;
            s.setName(dlg.getName());
            s.setDifficulty(dlg.getDifficulty());
            s.setImportance(dlg.getImportance());
            vector<string> topics = dlg.getTopics();
            s.setTopicsList(topics);
            subjects.push_back(s);
            refreshSubjectTable();
        }
    }

    void onRemoveSubject() {
        int r = subjectTable->currentRow();
        if (r >= 0 && r < (int)subjects.size()) {
            subjects.erase(subjects.begin() + r);
            refreshSubjectTable();
        }
    }

    void onGenerate() {
        if (subjects.empty()) {
            QMessageBox::warning(this, "No subjects", "Please add at least one subject.");
            return;
        }
        int days = daysSpin->value();
        double hoursPerDay = hoursSpin->value();

        ScheduleGenerator gen(days, hoursPerDay);
        vector<Subject> copySubs = subjects;
        gen.setSubjects(copySubs);
        gen.generateSchedule();

        lastSchedule = gen.getSchedule();

        analyzeHighlights();
        populateScheduleTable(lastSchedule);
        refreshSubjectTable();
    }

    void onSave() {
        QString fname = QFileDialog::getSaveFileName(this, "Save CSV", "study_schedule.csv", "CSV Files (*.csv)");
        if (fname.isEmpty()) return;

        if (!saveCsv(fname))
            QMessageBox::warning(this, "Save failed", "Could not save file.");
        else
            QMessageBox::information(this, "Saved", "Schedule saved to " + fname);
    }

    void onClearSchedule() {
        scheduleTable->setRowCount(0);
        subjectTable->setRowCount(0);
        subjects.clear();
        lastSchedule.clear();
        highlightReasons.clear();
        subjectHighlightReasons.clear();
    }

    void onDaysChanged(int newDays) {
        updateHighlightSpinRange();
    }

    void onFilterChanged(int index) {
        switch(index) {
            case 0: currentFilter = HighlightFilter::All; break;
            case 1: currentFilter = HighlightFilter::DifficultyOnly; break;
            case 2: currentFilter = HighlightFilter::TopicsOnly; break;
            case 3: currentFilter = HighlightFilter::HoursOnly; break;
        }
        if (!lastSchedule.empty()) {
            populateScheduleTable(lastSchedule);
            refreshSubjectTable();
        }
    }

private:
    // Highlight reasons enum
    enum HighlightReason { Difficulty, Topics, Hours };

    enum class HighlightFilter {
        All,
        DifficultyOnly,
        TopicsOnly,
        HoursOnly
    };

    HighlightFilter currentFilter;

    QSpinBox *daysSpin;
    QDoubleSpinBox *hoursSpin;
    QTableWidget *subjectTable;
    QTableWidget *scheduleTable;
    QComboBox *filterCombo;

    vector<Subject> subjects;
    vector<vector<Task>> lastSchedule;

    // Highlight data structures
    vector<set<HighlightReason>> highlightReasons; // per day
    vector<set<HighlightReason>> subjectHighlightReasons; // per subject index

    void updateHighlightSpinRange() {
        int d = daysSpin->value();
    }

    QString formatTime(double hours) const {
        int h = (int)hours;
        int m = (int)((hours - h) * 60 + 0.5);
        if (h == 0 && m > 0)
            return QString("%1 min").arg(m);
        if (m == 0)
            return QString("%1h").arg(h);
        return QString("%1h %2m").arg(h).arg(m, 2, 10, QChar('0'));
    }

    // Filter reasons based on current filter
    set<HighlightReason> filteredReasons(const set<HighlightReason>& reasons) const {
        switch (currentFilter) {
            case HighlightFilter::All:
                return reasons;
            case HighlightFilter::DifficultyOnly:
                return reasons.count(Difficulty) ? set<HighlightReason>{Difficulty} : set<HighlightReason>{};
            case HighlightFilter::TopicsOnly:
                return reasons.count(Topics) ? set<HighlightReason>{Topics} : set<HighlightReason>{};
            case HighlightFilter::HoursOnly:
                return reasons.count(Hours) ? set<HighlightReason>{Hours} : set<HighlightReason>{};
        }
        return reasons;
    }

    void analyzeHighlights() {
        int days = (int)lastSchedule.size();
        highlightReasons.clear();
        highlightReasons.resize(days);

        // Calculate sums for each day
        vector<int> difficultySum(days, 0);
        vector<int> topicCount(days, 0);
        vector<double> hoursSum(days, 0.0);

        for (int d = 0; d < days; ++d) {
            for (const Task &task : lastSchedule[d]) {
                // Find subject difficulty
                auto it = find_if(subjects.begin(), subjects.end(), [&](const Subject &s){ return s.getName() == task.subject; });
                int diff = (it != subjects.end()) ? it->getDifficulty() : 1;
                difficultySum[d] += diff;
                topicCount[d]++;
                hoursSum[d] += task.hours;
            }
        }

        // Find max values
        int maxDifficulty = *max_element(difficultySum.begin(), difficultySum.end());
        int maxTopics = *max_element(topicCount.begin(), topicCount.end());
        double maxHours = *max_element(hoursSum.begin(), hoursSum.end());

        // Mark highlight reasons per day
        for (int d = 0; d < days; ++d) {
            if (difficultySum[d] == maxDifficulty) highlightReasons[d].insert(Difficulty);
            if (topicCount[d] == maxTopics) highlightReasons[d].insert(Topics);
            if (abs(hoursSum[d] - maxHours) < EPSILON) highlightReasons[d].insert(Hours);
        }

        // Analyze subject highlights: for each subject, find days where it appears and reasons that day has
        subjectHighlightReasons.clear();
        subjectHighlightReasons.resize(subjects.size());

        for (size_t si = 0; si < subjects.size(); ++si) {
            const string &subjName = subjects[si].getName();
            set<HighlightReason> subjReasons;
            for (int d = 0; d < days; ++d) {
                bool subjectOnDay = false;
                for (const Task &task : lastSchedule[d]) {
                    if (task.subject == subjName) {
                        subjectOnDay = true;
                        break;
                    }
                }
                if (subjectOnDay) {
                    // Add filtered reasons of that day to subject reasons
                    for (HighlightReason r : highlightReasons[d])
                        subjReasons.insert(r);
                }
            }
            subjectHighlightReasons[si] = subjReasons;
        }
    }

    QColor colorForReason(const set<HighlightReason> &reasons) const {
        // We use bright strong colors, handle overlapping combinations by combining colors or using predefined ones
        bool diff = reasons.count(Difficulty);
        bool top = reasons.count(Topics);
        bool hrs = reasons.count(Hours);

        if (diff && top && hrs) return QColor("#800080"); // Purple all combined
        if (diff && top) return QColor("#FF4500"); // OrangeRed
        if (diff && hrs) return QColor("#FF8C00"); // DarkOrange
        if (top && hrs) return QColor("#1E90FF"); // DodgerBlue

        if (diff) return QColor("#FF0000"); // Red
        if (top) return QColor("#FFA500"); // Orange
        if (hrs) return QColor("#0000FF"); // Blue

        return QColor(); // no color
    }

    QString reasonsText(const set<HighlightReason> &reasons) const {
        QStringList parts;
        for (HighlightReason r : reasons) {
            switch (r) {
                case Difficulty: parts << "Highest Difficulty sum"; break;
                case Topics: parts << "Most Topics covered"; break;
                case Hours: parts << "Most Study Hours"; break;
            }
        }
        return parts.join(", ");
    }

    void populateScheduleTable(const vector<vector<Task>> &schedule) {
        scheduleTable->clearContents();
        scheduleTable->setRowCount(0);
        int row = 0;
        for (size_t d = 0; d < schedule.size(); ++d) {
            for (const Task &t : schedule[d]) {
                scheduleTable->insertRow(row);
                QTableWidgetItem *dayItem = new QTableWidgetItem(QString::number(d+1));
                QTableWidgetItem *subjectItem = new QTableWidgetItem(QString::fromStdString(t.subject));
                QTableWidgetItem *topicItem = new QTableWidgetItem(QString::fromStdString(t.topic));
                QTableWidgetItem *timeItem = new QTableWidgetItem(formatTime(t.hours));

                // Set contrasting text colors
                QBrush bgBrush = QBrush(Qt::white);

                // Highlight day row background by combined reason filter
                set<HighlightReason> reasons = highlightReasons[d];
                set<HighlightReason> filtered = filteredReasons(reasons);

                QColor bgColor = colorForReason(filtered);
                if (bgColor.isValid()) {
                    bgBrush = QBrush(bgColor);
                    // set text color contrast: if bg is dark, text white else black
                    QColor textColor = (bgColor.lightness() < 128) ? QColor(Qt::white) : QColor(Qt::black);
                    dayItem->setForeground(textColor);
                    subjectItem->setForeground(textColor);
                    topicItem->setForeground(textColor);
                    timeItem->setForeground(textColor);
                } else {
                    QColor textColor = QColor("#001f3f"); // dark navy for text
                    dayItem->setForeground(textColor);
                    subjectItem->setForeground(textColor);
                    topicItem->setForeground(textColor);
                    timeItem->setForeground(textColor);
                }
                dayItem->setBackground(bgBrush);
                subjectItem->setBackground(bgBrush);
                topicItem->setBackground(bgBrush);
                timeItem->setBackground(bgBrush);

                // Tooltip for day reasons
                if (!filtered.empty()) {
                    QString tooltip = QString("Day %1 highlight reason(s): %2").arg(d+1).arg(reasonsText(filtered));
                    dayItem->setToolTip(tooltip);
                }

                scheduleTable->setItem(row, 0, dayItem);
                scheduleTable->setItem(row, 1, subjectItem);
                scheduleTable->setItem(row, 2, topicItem);
                scheduleTable->setItem(row, 3, timeItem);
                ++row;
            }
        }
    }

    void refreshSubjectTable() {
        subjectTable->clearContents();
        subjectTable->setRowCount((int)subjects.size());
        for (size_t i = 0; i < subjects.size(); ++i) {
            const Subject &s = subjects[i];
            QTableWidgetItem *nameItem = new QTableWidgetItem(QString::fromStdString(s.getName()));
            QTableWidgetItem *diffItem = new QTableWidgetItem(QString::number(s.getDifficulty()));
            QTableWidgetItem *impItem = new QTableWidgetItem(QString::number(s.getImportance()));
            QTableWidgetItem *topicsItem = new QTableWidgetItem(QString::number(s.getTopicsCount()));

            set<HighlightReason> reasons = subjectHighlightReasons.size() > i ? subjectHighlightReasons[i] : set<HighlightReason>{};
            set<HighlightReason> filtered = filteredReasons(reasons);

            QColor bgColor = colorForReason(filtered);
            if (bgColor.isValid()) {
                QBrush bgBrush(bgColor);
                nameItem->setBackground(bgBrush);
                diffItem->setBackground(bgBrush);
                impItem->setBackground(bgBrush);
                topicsItem->setBackground(bgBrush);

                QColor textColor = (bgColor.lightness() < 128) ? QColor(Qt::white) : QColor(Qt::black);
                nameItem->setForeground(textColor);
                diffItem->setForeground(textColor);
                impItem->setForeground(textColor);
                topicsItem->setForeground(textColor);

                QString tooltip = QString("Subject highlight reason(s): %1").arg(reasonsText(filtered));
                nameItem->setToolTip(tooltip);
                diffItem->setToolTip(tooltip);
                impItem->setToolTip(tooltip);
                topicsItem->setToolTip(tooltip);
            } else {
                QColor textColor = QColor("#001f3f"); // dark navy text
                nameItem->setForeground(textColor);
                diffItem->setForeground(textColor);
                impItem->setForeground(textColor);
                topicsItem->setForeground(textColor);
            }

            subjectTable->setItem((int)i, 0, nameItem);
            subjectTable->setItem((int)i, 1, diffItem);
            subjectTable->setItem((int)i, 2, impItem);
            subjectTable->setItem((int)i, 3, topicsItem);
        }
    }

    bool saveCsv(const QString &filename) {
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;

        QTextStream out(&file);
        out << "Day,Subject,Topic,Time\n";
        for (size_t d = 0; d < lastSchedule.size(); ++d) {
            for (const Task &t : lastSchedule[d]) {
                out << QString::number(d+1) << ","
                    << QString::fromStdString(t.subject) << ","
                    << QString::fromStdString(t.topic) << ","
                    << formatTime(t.hours) << "\n";
            }
        }
        file.close();
        return true;
    }
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
