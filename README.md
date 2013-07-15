#-------------------------------------------------------------------------------
# Copyright: CodeChix 2013
# Author   : deepadhurka 
# Date     : 12 May 2013 
#-------------------------------------------------------------------------------

Objective
--------------------------------------------------------------------------------
Working, demonstration-ready code for submission to:
https://www.opennetworking.org/competition

Programming Language and Coding Standard
--------------------------------------------------------------------------------
C with some scripting where necessary
GNU Coding Standards (http://www.gnu.org/prep/standards/standards.pdf)

Branching
--------------------------------------------------------------------------------
2 branches are maintained in upstream:
1. master - code-reviewed, unit-tested merges from dev-onf-driver branch
2. dev-onf-driver - dev branch for forking and pull requests by contributors

Suggested workflow for contributors:
1. Fork the repo from your github account. 
   You will have forked both the master and the dev-onf-driver branches.

2. Clone your local repo from your github account to your local unix/linux 
dev environment 
Git commands to achieve this:
a.  git clone https://github.com/<your github user id>/CC-ONF-driver.git
b.  git checkout -b dev-onf-driver origin/dev-onf-driver
Branch dev-onf-driver set up to track remote branch dev-onf-driver from origin.
Switched to a new branch 'dev-onf-driver'

   Note: our working branch is dev-onf-driver. So any further branching MUST
         be done off of dev-onf-driver branch and merged back eventually.

   You can now branch it all you like - this will not impact upstream.
   Code, commit, push to your repo until ready to merge with upstream.
   Again, as a final step, merge with your dev-onf-driver branch BEFORE 
   pull request.

3. Initiate a 'pull request' via github online account for
   a. Initiating code review
   b. and once reviewed, to get merged with upstream dev-onf-driver branch


Banner for all new files:
#-------------------------------------------------------------------------------
# Copyright: CodeChix 2013
# Author   : <github account name>
# Date     : <date of file creation>
#-------------------------------------------------------------------------------

Footnote:
Please email deepa.karnad@gmail.com for any issues with info published in this file
