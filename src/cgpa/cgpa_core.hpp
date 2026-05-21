#pragma once

#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

namespace cgpa
{
struct CourseResult
{
    std::wstring name;
    int credits = 0;
    std::wstring grade;
    double gradePoint = 0.0;
};

struct Summary
{
    int courseCount = 0;
    int totalCredits = 0;
    double totalGradePoints = 0.0;
    double gpa = 0.0;
    std::wstring overallGrade;
};

inline double gradePointFor(const std::wstring& grade)
{
    if (grade == L"A" || grade == L"A+")
    {
        return 4.0;
    }
    if (grade == L"A-")
    {
        return 3.7;
    }
    if (grade == L"B+")
    {
        return 3.3;
    }
    if (grade == L"B")
    {
        return 3.0;
    }
    if (grade == L"B-")
    {
        return 2.7;
    }
    if (grade == L"C+")
    {
        return 2.3;
    }
    if (grade == L"C")
    {
        return 2.0;
    }
    if (grade == L"C-")
    {
        return 1.7;
    }
    if (grade == L"D+")
    {
        return 1.3;
    }
    if (grade == L"D")
    {
        return 1.0;
    }
    return 0.0;
}

inline std::wstring overallGradeFor(double gpa)
{
    if (gpa >= 3.85)
    {
        return L"A";
    }
    if (gpa >= 3.5)
    {
        return L"A-";
    }
    if (gpa >= 3.15)
    {
        return L"B+";
    }
    if (gpa >= 2.85)
    {
        return L"B";
    }
    if (gpa >= 2.5)
    {
        return L"B-";
    }
    if (gpa >= 2.15)
    {
        return L"C+";
    }
    if (gpa >= 1.85)
    {
        return L"C";
    }
    if (gpa >= 1.5)
    {
        return L"C-";
    }
    if (gpa >= 1.15)
    {
        return L"D+";
    }
    if (gpa >= 0.75)
    {
        return L"D";
    }
    return L"F";
}

inline Summary summarize(const std::vector<CourseResult>& courses)
{
    Summary summary{};
    summary.courseCount = static_cast<int>(courses.size());
    summary.totalCredits = std::accumulate(courses.begin(), courses.end(), 0, [](int total, const CourseResult& course) {
        return total + std::max(0, course.credits);
    });

    summary.totalGradePoints = std::accumulate(courses.begin(), courses.end(), 0.0, [](double total, const CourseResult& course) {
        return total + (static_cast<double>(std::max(0, course.credits)) * course.gradePoint);
    });

    if (summary.totalCredits > 0)
    {
        summary.gpa = summary.totalGradePoints / static_cast<double>(summary.totalCredits);
    }

    summary.overallGrade = overallGradeFor(summary.gpa);
    return summary;
}
}
