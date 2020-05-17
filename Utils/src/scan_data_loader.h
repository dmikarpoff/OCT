#pragma once

#include <QFile>
#include <QTextStream>

#include <exception>
#include <sstream>
#include <vector>

namespace oct {
namespace helpers
{

std::vector<double> loadFromCsv(QFile& file, size_t* width, size_t* height)
{
    QTextStream stream(&file);
    if (stream.atEnd()) {
        QString msg = "Input file '%1' should not be empty";
        throw std::runtime_error(msg.arg(file.fileName()).toStdString());
    }
    {
        QString first_line = stream.readLine();
        auto column_list = first_line.split(',');
        std::runtime_error err("Failed to parse width and height");
        if (column_list.size() != 4)
        {
            throw err;
        }
        bool width_ok = false;
        bool height_ok = false;
        *width = column_list[1].toULongLong(&width_ok);
        *height = column_list[3].toULongLong(&height_ok);
        if (!(width_ok && height_ok))
        {
            throw err;
        }
    }
    std::vector<double> res;
    res.reserve(*width * *height);
    for (size_t h = 0; h < *height; ++h)
    {
        if (stream.atEnd())
        {
            throw std::runtime_error("Expected new line, but not found");
        }
        auto column_list = stream.readLine().split(',');
        if (column_list.size() != *width)
        {
            std::stringstream msg;
            msg << "Expected " << *width << " columns, got " << column_list.size();
            throw std::runtime_error(msg.str());
        }
        for (size_t w = 0; w < *width; ++w)
        {
            bool ok = false;
            auto val = column_list[static_cast<int>(w)].toDouble(&ok);
            if (!ok)
            {
                throw std::runtime_error("Expected to read floating point value from cell");
            }
            res.push_back(val);
        }
    }

    return res;
}


}  // namespace helpers
}  // namespace oct
