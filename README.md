#IGOS Nusantara Software Development Kit (IGNSDK) [![Build Status](https://travis-ci.org/ubunteroz/ignsdk-debian.png)](https://travis-ci.org/ubunteroz/ignsdk-debian)
##Author
Eka Tresna Irawan/Ibnu Yahya (anak10thn@gmail.com)

##Contributors
###Wiki
* Eko Yunianto (eyeyunianto@gmail.com)
* Arif Eko Pratono (arifep2000@gmail.com)

###Packagers
* Debian, Ubuntu: Surya Handika Putratama (ubunteroz@gmail.com)
* Archlinux: Sofyan Saputra a.k.a Iank (meongbego@gmail.com)
* Slackware: Dwiyan Galuh W. a.k.a wajatmaka (wajatmaka@gmail.com)

##How To
###Build using Pbuilder
* `$ sudo aptitude install pbuilder`
* `$ sudo pbuilder create --distribution testing --debootstrapopts --variant=buildd`
* `$ cd ignsdk-debian`
* `$ pdebuild --use-pdebuild-internal`
