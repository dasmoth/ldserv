Simple LD server
================

Check out with `git clone --recursive` to pick up dependencies (currently just tabixpp), then
build with `make`.

For performance reasons, genotypes are stored in a simplified ("genotab") format, then indexed with
`tabix`.

             gzip -dc genotypes.vcf.gz | vcf2gt >genotypes.gt
             bgzip genotypes.gt
             tabix -p vcf genotypes.gt.gz

The `ldserv` program itself is intended to be run as a CGI script.  It consults the `GENOTYPE_FILE`
environment variable to locate the (Tabix-indexed) genotypes.  If using Apache, you can configure
this using something like:

        SetEnv GENOTYPE_FILE /path/to/genotypes.gt.gz
        ScriptAlias /cgi-bin/ /path/to/directory-containing-ldserv
