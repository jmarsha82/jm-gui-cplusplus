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
}

int main()
{
    if (!near(cgpa::gradePointFor(L"A"), 4.0) || !near(cgpa::gradePointFor(L"B+"), 3.3) || !near(cgpa::gradePointFor(L"F"), 0.0))
    {
        return fail("gradePointFor returned an unexpected grade point value.");
    }

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

    const cgpa::Summary emptySummary = cgpa::summarize({});
    if (emptySummary.courseCount != 0 || emptySummary.totalCredits != 0 || !near(emptySummary.gpa, 0.0) || emptySummary.overallGrade != L"F")
    {
        return fail("Empty summary returned unexpected values.");
    }

    return 0;
}
