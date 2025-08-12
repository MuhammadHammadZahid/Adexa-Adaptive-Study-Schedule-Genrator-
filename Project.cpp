// main.cpp
// Qt Study Schedule Generator
// Single-file Qt Widgets application

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

#include <vector>
#include <string>
#include <queue>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <stdexcept>

using namespace std;

// ====== Data Models ======
class Subject {
private:
    string name;
    int difficulty;      // 1 to 10
    int importance;      // 1 to 10
    int topics;
    double remainingHours;
    vector<string> topicsList;

public:
    Subject() : name(""), difficulty(1), importance(1), topics(0), remainingHours(0.0) {}

    Subject(const string &n, int diff, int imp, int t, const vector<string> &topicNames)
        : name(n), difficulty(diff), importance(imp), topics(t), remainingHours(0.0), topicsList(topicNames) {}

    // Removed calculateRequiredHours - we won't use base multiplier now.

    string getName() const { return name; }
    int getDifficulty() const { return difficulty; }
    int getImportance() const { return importance; }
    int getTopicsCount() const { return topics; }
    double getRemainingHours() const { return remainingHours; }
    void setRemainingHours(double hrs) { remainingHours = hrs; }

    bool hasTopics() const { return !topicsList.empty(); }

    // Access topic cyclically by index
    string getTopicAtIndex(size_t idx) const {
        if (topicsList.empty()) return string("Topic");
        return topicsList[idx % topicsList.size()];
    }

    void addTopic(const string &t) { topicsList.push_back(t); }
    void setName(const string &n) { name = n; }
    void setDifficulty(int d) { difficulty = d; }
    void setImportance(int i) { importance = i; }
    void setTopics(int t) { topics = t; }
};

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
        // Calculate weights for each subject: difficulty * importance * topics count
        vector<double> weights;
        double totalWeight = 0.0;
        for (auto &s : subjects) {
            double w = s.getDifficulty() * s.getImportance() * max(1, s.getTopicsCount());
            weights.push_back(w);
            totalWeight += w;
        }

        double totalAvailableHours = days * hoursPerDay;

        // Assign hours proportional to weights
        for (int i = 0; i < (int)subjects.size(); ++i) {
            double assignedHours = (totalWeight > 0) ? (weights[i] / totalWeight) * totalAvailableHours : 0;
            subjects[i].setRemainingHours(assignedHours);
        }

        // Clear previous schedule and resize
        schedule.clear();
        schedule.resize(days);

        // Track topic indices for cyclic repetition
        vector<size_t> topicIndices(subjects.size(), 0);

        // Distribute time day-by-day
        for (int d = 0; d < days; ++d) {
            double left = hoursPerDay;

            bool assignedSomething = true;

            while (left > 0.01 && assignedSomething) {
                assignedSomething = false;

                for (int i = 0; i < (int)subjects.size() && left > 0.01; ++i) {
                    if (subjects[i].getRemainingHours() <= 0) continue;

                    double toAssign = min(left, min(subjects[i].getRemainingHours(), 2.0)); // max chunk 2 hours

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

// ====== Add Subject Dialog ======
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

// ====== Main Window ======
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        QWidget *central = new QWidget;
        QVBoxLayout *mainLayout = new QVBoxLayout;

        // Controls
        QGroupBox *controlsBox = new QGroupBox("Schedule Settings");
        QFormLayout *controlsLayout = new QFormLayout;

        daysSpin = new QSpinBox; daysSpin->setRange(1,365); daysSpin->setValue(14);
        hoursSpin = new QDoubleSpinBox; hoursSpin->setRange(0.5,24.0); hoursSpin->setSingleStep(0.5); hoursSpin->setValue(4.0);

        controlsLayout->addRow("Days:", daysSpin);
        controlsLayout->addRow("Hours per day:", hoursSpin);
        controlsBox->setLayout(controlsLayout);

        // Subject table
        subjectTable = new QTableWidget(0,4);
        subjectTable->setHorizontalHeaderLabels({"Name","Difficulty","Importance","#Topics"});
        subjectTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

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
        mainLayout->addWidget(controlsBox);
        mainLayout->addWidget(new QLabel("Subjects"));
        mainLayout->addWidget(subjectTable);
        mainLayout->addLayout(subjectBtns);
        mainLayout->addLayout(actionBtns);
        mainLayout->addWidget(new QLabel("Generated Schedule"));
        mainLayout->addWidget(scheduleTable);

        central->setLayout(mainLayout);
        setCentralWidget(central);
        setWindowTitle("Study Schedule Generator");
        resize(800, 700);

        // Connections
        connect(addSubjectBtn, &QPushButton::clicked, this, &MainWindow::onAddSubject);
        connect(removeSubjectBtn, &QPushButton::clicked, this, &MainWindow::onRemoveSubject);
        connect(generateBtn, &QPushButton::clicked, this, &MainWindow::onGenerate);
        connect(saveBtn, &QPushButton::clicked, this, &MainWindow::onSave);
        connect(clearBtn, &QPushButton::clicked, this, &MainWindow::onClearSchedule);
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
            for (auto &t : topics) s.addTopic(t);
            s.setTopics((int)topics.size());
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

        populateScheduleTable(gen.getSchedule());
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
    }

private:
    // UI Widgets
    QSpinBox *daysSpin;
    QDoubleSpinBox *hoursSpin;
    QTableWidget *subjectTable;
    QTableWidget *scheduleTable;

    // Data
    vector<Subject> subjects;
    vector<vector<Task>> lastSchedule;

    void refreshSubjectTable() {
        subjectTable->setRowCount(0);
        for (size_t i = 0; i < subjects.size(); ++i) {
            subjectTable->insertRow((int)i);
            subjectTable->setItem((int)i, 0, new QTableWidgetItem(QString::fromStdString(subjects[i].getName())));
            subjectTable->setItem((int)i, 1, new QTableWidgetItem(QString::number(subjects[i].getDifficulty())));
            subjectTable->setItem((int)i, 2, new QTableWidgetItem(QString::number(subjects[i].getImportance())));
            subjectTable->setItem((int)i, 3, new QTableWidgetItem(QString::number(subjects[i].getTopicsCount())));
        }
    }

    // Helper to format time (hours in double) to "Xh Ym"
    QString formatTime(double hours) {
        int h = (int)hours;
        int m = (int)((hours - h) * 60 + 0.5);
        return QString("%1h %2m").arg(h).arg(m, 2, 10, QChar('0'));
    }

    void populateScheduleTable(const vector<vector<Task>> &sched) {
        scheduleTable->setRowCount(0);
        lastSchedule = sched;
        for (size_t d = 0; d < sched.size(); ++d) {
            for (size_t t = 0; t < sched[d].size(); ++t) {
                int row = scheduleTable->rowCount();
                scheduleTable->insertRow(row);
                scheduleTable->setItem(row, 0, new QTableWidgetItem(QString::number((int)d + 1)));
                scheduleTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(sched[d][t].subject)));
                scheduleTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(sched[d][t].topic)));
                scheduleTable->setItem(row, 3, new QTableWidgetItem(formatTime(sched[d][t].hours)));
            }
        }
    }

    bool saveCsv(const QString &path) {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
        QTextStream out(&file);
        out << "Day,Subject,Topic,Time\n";
        for (int r = 0; r < scheduleTable->rowCount(); ++r) {
            out << scheduleTable->item(r,0)->text() << ","
                << scheduleTable->item(r,1)->text() << ","
                << scheduleTable->item(r,2)->text() << ","
                << scheduleTable->item(r,3)->text() << "\n";
        }
        file.close();
        return true;
    }
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
