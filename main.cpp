/*
 * Copyright (C) 2013  Phil Hannent
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QXmlStreamWriter>
#include <zlib.h>

using namespace std;

QStringList file_ext;

//TODO: Read existing file to update
//TODO: Add ignore list

void
list_files(const QString current_path,
           QXmlStreamWriter &stream,
           const QString base_path)
{
    QDir dir(current_path);

    if (dir.exists())
    {
        QStringList folder_files = dir.entryList(QDir::Files);
        /* Add all of the files */
        foreach (QString file_name, folder_files) {
            QFileInfo fi(current_path + QDir::separator() + file_name);
            if (file_ext.indexOf(fi.suffix().toLower()) >= 0) {
                stream.writeStartElement("url");
                QString output_folder = current_path;
                output_folder.replace(base_path, "");
                /* Remove the root marker */
                if (output_folder.left(1) == "/"
                        || output_folder.left(1) == "\\")
                    output_folder.remove(0,1);
                /* Add the last folder marker */
                if (!output_folder.isEmpty()
                        && output_folder.right(1) != "/")
                    output_folder += "/";
                /* Output all the information we have */
                stream.writeTextElement("loc", output_folder + file_name);
                stream.writeTextElement("lastmod", fi.lastModified().toString(Qt::ISODate));
                stream.writeTextElement("changefreq", "daily");
                stream.writeTextElement("priority", "0.5");
                stream.writeEndElement(); /* end of the Url element */
            }
        }

        QStringList folder_folders = dir.entryList(
                    QDir::Dirs | QDir::NoDotAndDotDot);
        /* Add all of the folders */
        foreach (QString folder_name, folder_folders) {
            list_files(current_path
                       + QDir::separator()
                       + folder_name,
                       stream,
                       base_path);
        }
    }
}


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Offline Sitemap Generator");
    QCoreApplication::setApplicationVersion("1.0");

    file_ext << "asp"
             << "aspx"
             << "cfm"
             << "do"
             << "htm"
             << "html"
             << "jhtml"
             << "jsp"
             << "jspx"
             << "php"
             << "php4"
             << "php3"
             << "phtml"
             << "py"
             << "rb"
             << "rhtml"
             << "rss"
             << "shtml"
             << "xhtml"
             << "yaws";

    /* Fetch folder to test and the name of the output file */
    QString source_path(QDir::currentPath());
    QString output_file("sitemap.xml");
    QCommandLineParser parser;
    parser.setApplicationDescription("Offline Sitemap Generator");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("source", QCoreApplication::translate("main", "Source file to copy."));
    parser.addPositionalArgument("destination", QCoreApplication::translate("main", "Destination directory."));

    QCommandLineOption ignore_option(QStringList() << "i" << "ignore"
        , QCoreApplication::translate("main",
            "Ignore files, use a comma separated list(not working yet)")
        , "ignore_option");
    parser.addOption(ignore_option);

    QCommandLineOption ext_option(QStringList() << "e" << "extensions"
        , QCoreApplication::translate("main",
            "Additional file extensions to include, sse a comma separated list")
        , "ext_option");
    parser.addOption(ext_option);
    parser.process(app);
    const QStringList args = parser.positionalArguments();
    source_path = args.at(0);

    QString add_extensions = parser.value(ext_option);
    if (!add_extensions.isEmpty())
    {
        qDebug() << "Addition Extensions:" << add_extensions;
        file_ext.append(add_extensions.split(","));
    }

    qDebug() << "The Path:" << source_path;
    QDir myPath( source_path);

    /* Check for valid path */
    if (!myPath.exists(".")) {
        qDebug() << "Base folder not found: " << myPath.canonicalPath();
        qApp->quit();
    }

    QFile myFile(source_path + QDir::separator() + output_file);
    /* Need to create the output file */
    if (!myFile.open(QIODevice::Truncate
                     | QIODevice::WriteOnly
                     | QIODevice::Text)) {
        qDebug() << "Could not open for write";
        qApp->quit();
    }
    QXmlStreamWriter stream(&myFile);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("urlset");
    stream.writeNamespace("http://www.sitemaps.org/schemas/sitemap/0.9");
    list_files(source_path, stream, source_path);
    stream.writeEndElement(); /* urlset */
    stream.writeEndDocument();
    myFile.flush();
    myFile.close();
    if (myFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream reader(&myFile);
        QString myData = reader.readAll();
        gzFile fi = (gzFile)gzopen("sitemap.xml.gz","wb");
        gzwrite(fi,myData.toLatin1(),myData.size());
        gzclose(fi);
    }

    qApp->quit();
    return 0;
}
