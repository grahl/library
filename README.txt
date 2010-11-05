Author:  Jess Straatmann
Released under the GPL


Description:
============
This module allows users to manage nodes as assets that may be available or unavailable. Users may create custom content types using CCK and then add those content types to the library.

It supports multiple copies of a library item associated with one node, and each copy may be individually made available or unavailable.

The module allows administrators to define their own library actions. Library actions may make an item available, unavailable, or not change an item's status. Every transaction is associated with a library patron. If you use the trigger module (part of core) with the library module, each library action generates a trigger that you can assign further actions to. The module includes a few built-in actions (send an email, extend the due date of an item).

The library module is intended to be very flexible and comes with a number of options. You may use unique identifiers on item instances (e.g. barcodes), have library patrons as users, and configure what fields display in the custom library search.

To get the full functionality of the module, you should have CCK installed and Triggers enabled.


Requirements:
=============
Drupal 6.x


Installation:
=============
Copy the entire library module directory to your module directory and enable the patron and library modules. 

After installing the patron and library modules, the site administrator needs to add content types to the library (to be managed as library items.)  The content types may be created by CCK.  To add a content type to the library, modify the content type settings at admin/content/node-type/<type>.  Select 'Yes' for Library Item under Workflow settings.  

If you have CCK installed, you may configure individual fields to display as columns in library lists/search results on the individual field configuration pages ( e.g. admin/content/node-type/<type>/fields/<field> )

Patrons:
=========
Patrons are Drupal users. Just assign the permissions you would like patrons to have by role.

Library Settings:
=========
Library module settings may be configured at admin/settings/library.  No options are enabled by default, so to see any additional functionality, users will need to modify the settings. Settings include:
#Use unique identifiers (barcodes) on library items
#Display specific actions as options
#Display specific fields in library lists
#Display terms from specific vocabularies in library lists
#Modify status text (e.g. "Available" vs. "IN")
#Enable due date functionality
#Add/rename library actions

The library module comes with two default library actions: 'Check In' and 'Check Out'.  You may rename these and/or create new library actions at admin/settings/library/actions.  

Every library action creates an individual trigger if the Trigger module is enabled.  Site administrators may assign further actions to occur with a specific library action (e.g send an email) by assigning them at admin/build/trigger/library.  Currently, only the library module provided actions are compatible with these triggers. 

Access Control:
===============
Each library action has it's own permission, so access control can be very fine-grained.  To give library patrons individual permissions, you will need to associate patrons with Drupal users on admin/settings/library/patrons. 
