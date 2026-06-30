#include "cgpa/cgpa_core.hpp"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace
{
bool near(double actual, double expected)
{
    return std::fabs(actual - expected) < 0.001;
}

int fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

int testGradePointMapping()
{
    const std::vector<std::pair<std::wstring, double>> cases = {
        {L"A+", 4.0},
        {L"A", 4.0},
        {L"A-", 3.7},
        {L"B+", 3.3},
        {L"B", 3.0},
        {L"B-", 2.7},
        {L"C+", 2.3},
        {L"C", 2.0},
        {L"C-", 1.7},
        {L"D+", 1.3},
        {L"D", 1.0},
        {L"F", 0.0},
        {L"not-a-grade", 0.0},
    };

    for (const auto& [grade, expected] : cases)
    {
        if (!near(cgpa::gradePointFor(grade), expected))
        {
            return fail("gradePointFor returned an unexpected grade point value.");
        }
    }

    return 0;
}

int testOverallGradeBoundaries()
{
    const std::vector<std::pair<double, std::wstring>> cases = {
        {3.85, L"A"},
        {3.50, L"A-"},
        {3.15, L"B+"},
        {2.85, L"B"},
        {2.50, L"B-"},
        {2.15, L"C+"},
        {1.85, L"C"},
        {1.50, L"C-"},
        {1.15, L"D+"},
        {0.75, L"D"},
        {0.74, L"F"},
    };

    for (const auto& [gpa, expected] : cases)
    {
        if (cgpa::overallGradeFor(gpa) != expected)
        {
            return fail("overallGradeFor returned an unexpected grade boundary value.");
        }
    }

    return 0;
}

int testSummaryCalculations()
{
    const std::vector<cgpa::CourseResult> courses = {
        {L"Math", 3, L"A", cgpa::gradePointFor(L"A")},
        {L"History", 4, L"B", cgpa::gradePointFor(L"B")},
        {L"Lab", 1, L"C+", cgpa::gradePointFor(L"C+")},
    };

    const cgpa::Summary summary = cgpa::summarize(courses);
    if (summary.courseCount != 3)
    {
        return fail("Summary did not count courses correctly.");
    }
    if (summary.totalCredits != 8)
    {
        return fail("Summary did not total credits correctly.");
    }
    if (!near(summary.totalGradePoints, 26.3))
    {
        return fail("Summary did not total grade points correctly.");
    }
    if (!near(summary.gpa, 3.2875))
    {
        return fail("Summary did not calculate GPA correctly.");
    }
    if (summary.overallGrade != L"B+")
    {
        return fail("Summary did not assign the expected overall grade.");
    }

    return 0;
}

int testSummaryIgnoresNegativeCredits()
{
    const std::vector<cgpa::CourseResult> courses = {
        {L"Valid", 3, L"A", 4.0},
        {L"Invalid", -4, L"A", 4.0},
    };

    const cgpa::Summary summary = cgpa::summarize(courses);
    if (summary.courseCount != 2)
    {
        return fail("Summary should still report every supplied course.");
    }
    if (summary.totalCredits != 3 || !near(summary.totalGradePoints, 12.0) || !near(summary.gpa, 4.0))
    {
        return fail("Summary should ignore negative credits when calculating totals.");
    }

    return 0;
}

int testEmptySummary()
{
    const cgpa::Summary emptySummary = cgpa::summarize({});
    if (emptySummary.courseCount != 0 || emptySummary.totalCredits != 0 || !near(emptySummary.gpa, 0.0) || emptySummary.overallGrade != L"F")
    {
        return fail("Empty summary returned unexpected values.");
    }

    return 0;
}
}

int main()
{
    if (const int result = testGradePointMapping(); result != 0) return result;
    if (const int result = testOverallGradeBoundaries(); result != 0) return result;
    if (const int result = testSummaryCalculations(); result != 0) return result;
    if (const int result = testSummaryIgnoresNegativeCredits(); result != 0) return result;
    if (const int result = testEmptySummary(); result != 0) return result;
    return 0;
}
