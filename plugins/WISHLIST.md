## Archive blk plugin

To avoid implementing exotic FS like PFS,MCFS,HDL, ROMFS ... client side
we could make a blk driver mimic an archive format like tar,
and we could later use compression like lz4 => mc0.tar.lz4
such standard archive have many tool to work with.

need:

 * a minimal VFS (ie using FUSE minimal API) to wrap exotic or virtual fs
 * tar support
 * filters support
