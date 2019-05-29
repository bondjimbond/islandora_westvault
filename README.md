# Islandora Westvault

## Introduction

Islandora Westvault lets you flag invididual objects, newspapers, books, or whole collections to be preserved in COPPUL's Westvault LOCKSS service 
or another OwnCloud-based service.

The user checks a box to add a PRESERVATION datastream to objects. On a cron run, flagged objects are identified, bagged, and their 
PRESERVATION datastreams updated to confirm that they have been preserved. Bags are moved to a specified directory in the filesystem. 
Then, on another cron run, that directory is synced with the repository's OwnCloud account.

Bags in OwnCloud are eventually sent to Westvault. At that point the OwnCloud directory is emptied, and the bags are cleared from 
the filesystem on the next sync.

## Requirements

The following Drupal modules are required:

 * [Islandora](https://github.com/islandora/islandora)
 * [Islandora Collection Solution Pack](https://github.com/islandora/islandora_solution_pack_collection)
 * [Islandora Solr Search](https://github.com/islandora/islandora_solr_search)
 * [Islandora Bag-It](https://github.com/islandora/islandora_bagit)
 * [Islandora Bag-It Complex](https://github.com/mjordan/islandora_bagit_complex)
 * [Islandora Westvault Extras](https://github.com/mjordan/islandora_westvault_extras)

External tools:

 * [OwnCloud client](https://software.opensuse.org/download/package?project=isv:ownCloud:desktop&package=owncloud-client)
 * A [WestVault](https://coppul.ca/westvault) account, or another OwnCloud based account where files will be preserved.

## Installation

[Install Drupal modules](https://drupal.org/documentation/install/modules-themes/modules-7) as usual. Git clone is recommended.

[Install OwnCloud client](https://software.opensuse.org/download/package?project=isv:ownCloud:desktop&package=owncloud-client) on your Islandora server. 

## Configuration

In Islandora:

* OwnCloud Local Path: Set a directory on the server where your Bags will be stored.
* OwnCloud WebDAV URL: URL to your OwnCloud sync directory. Should end in '/lockss-preserved' if using WestVault.
* OwnCloud user ID and password: Self-explanatory.

In OwnCloud:

* In your WestVault settings (Files->WestVault->Settings), check "Remove completed deposits" to keep your directory clean.

Configuring Islandora Bag-it:

* Change the Collection Batch Type settings to "Collection object only". Islandora Westvault works by preserving individual objects; collection bags with multiple objects in them may cause problems such as duplication in your LOCKSS account.
* Under "Complex Objects", select Books and Newspaper Issues. (Do not select Newspapers or Compound Objects; Islandora Westvault handles their children individually.)
* Check the "Add WestVault Tags" option.

On your webserver:

* Make sure the directory /tmp/islandora_bagit_tmp is writable.

## How to use

### Flagging objects for preservation

In Islandora, Manage the collection, newspaper, or other object that you want preserved. Check the box under the Preservation tab.

For Collections, three options are offered for dealing with collection children:

* All children: Preserves all of the collection's children, and all future children on ingest.
* New children: Ignores current children of the colleciton, but all future children will be preserved.
* Current children only: Preserves all current children, but any future children added will not be preserved.

If a new Collection is ingested into a preserved collection, the new collection does *not* automatically get preserved. It will need to be tagged manually.

If a Newspaper object is preserved, all child issues of that newspaper will also be preserved, and any future child issues. 
All Pages of preserved Newspaper Issue objects and Books are also preserved.

### Bagging objects

Once the module is configured and some objects have been flagged for preservation, set up Cron jobs to generate bags and sync with OwnCloud.

In crontab, set a job to run the command "drush westvault-bagit" at some interval, e.g. once per day. 

Example:

`export PATH=$PATH:/usr/bin/drush 30 0 * * * drush -u 1 westvault-bagit`

The above runs the Drush script every day at 12:30 AM. This script finds all objects with a PRESERVATION datastream, checks whether they have been preserved before,
then runs Islandora Bagit against the resulting list of objects. Bags are created and then sent to the directory specified in the module configuration.

### Sending bags to OwnCloud

In crontab, set a job to run the command "drush westvault-sync" once per day. This should be set some time later than the westvault-bagit command 
to ensure that they don't overlap.

`export PATH=$PATH:/usr/bin/drush 30 4 * * * drush -u 1 westvault-sync`

## Multisites

If using this tool in a multisite environment, some additional considerations must be made.

### OwnCloud Local Path

Make sure that each site uses a different directory here. Otherwise, bags might be sent to the wrong account when synced.

### Crontab

Run separate Cron jobs for each site so that Drush executes in different directories. For example, if you have three child sites: 

- 30 0 * * * cd /var/www/drupal/sites/site1.myrepo.ca && drush -u 1 westvault-bagit
- 30 1 * * * cd /var/www/drupal/sites/site2.myrepo.ca && drush -u 1 westvault-bagit
- 30 2 * * * cd /var/www/drupal/sites/site3.myrepo.ca && drush -u 1 westvault-bagit
- 30 10 * * * cd /var/www/drupal/sites/site1.myrepo.ca && drush -u 1 westvault-sync
- 30 11 * * * cd /var/www/drupal/sites/site2.myrepo.ca && drush -u 1 westvault-sync
- 30 12 * * * cd /var/www/drupal/sites/site3.myrepo.ca && drush -u 1 westvault-sync

The above will execute the Bagit and Sync commands in each child site separately, so that their individual settings will be used to carry out the process (OwnCloud accounts, bag storage directories, etc.)

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

## Maintainers/Sponsors

Current maintainers:

* [Brandon Weigel](https://github.com/bondjimbond)

## Development

If you would like to contribute to this module, please check out our helpful [Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers) info, as well as our [Developers](http://islandora.ca/developers) section on the Islandora.ca site.

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
