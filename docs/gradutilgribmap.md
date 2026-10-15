---
title: gribmap
---

# **gribmap**

The GrADS data descriptor file defines a grid structure into which the data will fit -- it gives "shape" to the data by identifying its spatial dimensions, the number of time steps, and the number of variables.

If the data is in binary format, its <a href="aboutgriddeddata.html#structure">structure</a> has already been prescribed. If the data is in GRIB format, no consistent relationship exists between the data and the grid structure defined in the data descriptor file. Hence, the need for the `gribmap` utility which "maps" between the GRIB data and the GrADS data description.

As `gribmap` reads each field in the GRIB data file, the parameters for that field (e.g. variable name, vertical level, time) are compared to the information in the data descriptor file until a match is found. The process continues until all the GRIB elements have been "mapped" to their location within the GrADS grid structure. Please read the documentation on <a href="grib.html">Handling GRIB in GrADS</a> for more details.

## Syntax

<table class="plaintext" width="680" data-border="0" data-cellpadding="5" data-cellspacing="5">
<colgroup>
<col style="width: 50%" />
<col style="width: 50%" />
</colgroup>
<tbody>
<tr data-bgcolor="c5d5ff">
<td colspan="2" data-bordercolor="#F4F4F4" data-bgcolor="#E0F0FF"><p><code>gribmap [</code><em><code>options</code></em><code>] </code></p>
<p>Command line <em><code>options</code></em> are:<br />
</p></td>
</tr>
<tr data-bgcolor="c5d5ff">
<td width="100"><code>-0</code></td>
<td>Ignores the forecast time when setting up a match, so that the base time is the valid time.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td width="100"><code>-b</code></td>
<td>Valid time for averages is set to be the beginning of the period rather than the end. Default valid time is the end of the averaging period<code>.</code></td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-big</code></td>
<td>(GrADS version 2.0.a8+) Required option if the GRIB1 or GRIB2 files are &gt; 2 Gb. This option creates a bigger index file, which cannot be read by earlier versions of GrADS.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-e</code><br />
<code>-E</code></td>
<td>Ignores extra bytes (that are not part of the GRIB1 msg) at end of file. Some ECMWF GRIB files require this option because of blocking. Using the <code>-E</code> option ignores extra bytes in middle and/or end of GRIB file.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td width="100"><code>-f </code><em><code>hr</code></em></td>
<td>Matches only those grib records whose forecast time is <em><code>hr</code></em> hours. This is used to isolate a sequence of forecasts. For example, if you wanted to sample all the 120-hour forecasts from the MRF ensemble runs, you would use <code>gribmap -f120</code>.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-h </code><em><code>num</code></em></td>
<td>Skips over <em><code>num</code></em> bytes before starting the scan process.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-help</code></td>
<td>Prints out the list of options</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-i </code><em><code>fname</code></em></td>
<td><em><code>fname</code></em> is the name of the data descriptor file. If not specified, <code>gribmap</code> will prompt the user.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-m</code></td>
<td>Use base time from descriptor file instead of GRIB header. This option only works with GRIB1.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-min0</code></td>
<td>Ignores the minutes code.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-N</code></td>
<td>Does NOT write out the index file.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-new</code></td>
<td>(GrADS version 2.1.a2+) Creates a new version of an existing index file</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-old</code></td>
<td>(GrADS version 2.1.a2+) Creates an old version of an existing index file</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-q</code></td>
<td>Quiet mode -- suppresses all messages except for errors. Default is off.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-s </code><em><code>num</code></em></td>
<td>Skips over no more than<em><code> num </code></em>bytes between records. The default is 1000.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-t0</code></td>
<td>Matches only those grib records whose base time is the same as the initial time in the data descriptor file. This is used to pull out a forecast sequence (0, 12, 24, ... , 72 hours) starting a specific time.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td width="100"><code>-u</code></td>
<td><p>Updates existing index file when expanding the size of the T or E dimensions in a <a href="templates.html">templated</a> data set. Only one dimension may be expanded at a time. This option was temporarily disabled in GrADS version 2.0.*, but is back in version 2.1.a2. (See Usage Notes and Examples below.)</p></td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-v</code></td>
<td>Verbose mode -- detailed output makes it easier to verify what is being mapped. Default is off.</td>
</tr>
</tbody>
</table>

 

### Usage Notes

Over the many years of GrADS development, the contents of the index file created by `gribmap` have changed only a few times. Although it is not present in the user interface, there is a version number associated with the index file. When the contents of the index are different and the version has been upgraded, you will get an error message when trying to read that file with an older version of GrADS. Index files created with `gribmap` version 2.1.a2+ have a new version number and therefore they can only be read by GrADS version 2.1.a2+. If you need to use an older version of GrADS to open a dataset with an index file created by `gribmap` version 2.1.a2+, you must first reformat the metadata in the index file by running `gribmap` again with the `-old` option.

The `-old` option (introduced with GrADS version 2.1.a2) allows you to downgrade the version of an index file so that it may be read by any version of GrADS. The `-old` option does not re-scan the GRIB files, it merely alters the metadata in the index file's header to match the old version. There are some cases when the `-old` option cannot be used: an index file that was created with the `-big` option cannot be downgraded with `-old` because that would require a change to the format of the *data* in the index file, not just the metadata in the index file's header.

The `-u` option was disabled in version 2.0\* because of the introduction of the E dimension, but has been restored in version 2.1.a2, so that now both the T and E dimensions can be updated. The EDEF and TDEF entries must be updated separately -- only one dimension can be expanded at a time. Reviving the update feature necessitated a change to the metadata in the index file's header, so the index version has changed with GrADS 2.1.a2.

If you have an old index file that you would like to update, first use the `-new` option to rewrite the index file with the additional metadata. The `-new` option does not re-scan the GRIB files, it merely upgrades the metadata in the index file's header.

 

### Example

Suppose you have an existing GRIB dataset (a reanalysis) that needs to have its time axis extended. To use the update feature without having to re-scan all files, follow these steps:

1\. Do not alter the descriptor file (yet). Run` gribmap -new `to upgrade the metadata in the index file and change the index version.\
2. Edit the descriptor file to expand the TDEF size. Do not change the name of the file in the INDEX entry.\
3. Run` gribmap -u `with the edited descriptor file.\
4. Run` gribmap -old `to downgrade the metadata in the new index file.

Step \#4 is optional, but it allows older versions of GrADS to read the updated data file. Without step \#4, only the newest version of GrADS (2.1.a2+) will be able to open the updated descriptor file.
