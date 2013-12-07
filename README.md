osmg
====

Offline Sitemap Generator

This command line tool is meant for use when automating your deployments. It
will itterate over a folder structure and produce a sitemap file for adding
to your upload.

This is most useful as a post checkout hook for git or before scripting your
uploads. Personally I've used it for creating the sitemap before uploading 
to a content delivery network that only supports flat files.
