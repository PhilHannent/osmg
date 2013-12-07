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
list_files(QString currentPath, QXmlStreamWriter &stream, QString basePath) {
    /* Now check each file and output the details, also create the md5 file */
    QDir dir(currentPath);

    QStringList knownFiles;
    if (dir.exists())
    {
        QStringList myFiles = dir.entryList(knownFiles, QDir::Files);
        /* Add all of the files */
        foreach (QString sFileName, myFiles) {
            QFileInfo fi(currentPath + QDir::separator() + sFileName);
            if (file_ext.indexOf(fi.suffix().toLower()) >= 0) {
                stream.writeStartElement("url");
                QString outputFolder = currentPath;
                outputFolder.replace(basePath, "");
                if (outputFolder.left(1) == "/" || outputFolder.left(1) == "\\")
                    outputFolder.remove(0,1);
                if (!outputFolder.isEmpty() && outputFolder.right(1) != "/")
                    outputFolder += "/";
                stream.writeTextElement("loc", outputFolder + sFileName);
                stream.writeTextElement("lastmod", fi.lastModified().toString(Qt::ISODate));
                stream.writeTextElement("changefreq", "daily");
                stream.writeTextElement("priority", "0.5");
                stream.writeEndElement(); /* Url */
            }
        }

        QStringList myFolders = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        /* Add all of the folders */
        foreach (QString sFolderName, myFolders) {
            list_files(currentPath + QDir::separator() + sFolderName, stream, basePath);
        }
    }
}


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Offline Sitemap Generator");
    QCoreApplication::setApplicationVersion("1.0");

    file_ext << "htm"
             << "php"
             << "asp"
             << "aspx"
             << "jsp"
             << "py"
             << "shtml";

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
    //stream.writeCharacters("<?xml version='1.0' encoding='UTF-8'?>");
    stream.writeStartElement("urlset");
    stream.writeNamespace("http://www.sitemaps.org/schemas/sitemap/0.9");
    //qDebug() << "about to check folders";
    list_files(source_path, stream, source_path);
    //qDebug() << "output the list to the file";
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
