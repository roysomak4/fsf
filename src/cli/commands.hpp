#pragma once

namespace fsf
{
    // Each subcommand gets argv with argv[0] being the subcommand name and returns the process exit code
    int cmd_map(int argc, char **argv);
    int cmd_prep_lib(int argc, char **argv);
    int cmd_multimap(int argc, char **argv);
    int cmd_predict(int argc, char **argv);
    int cmd_filter(int argc, char **argv);
    int cmd_ffpm(int argc, char **argv);
    int cmd_splice_info(int argc, char **argv);
    int cmd_annotate(int argc, char **argv);
    int cmd_annot_filter(int argc, char **argv);
    int cmd_rt_filter(int argc, char **argv);
    int cmd_min_ffpm(int argc, char **argv);
    int cmd_column_exclusions(int argc, char **argv);
    int cmd_coding_effect(int argc, char **argv);
    int cmd_evidence_reads(int argc, char **argv);
    int cmd_fi_contigs(int argc, char **argv);
    int cmd_fi_star(int argc, char **argv);
    int cmd_fi_dedup(int argc, char **argv);
    int cmd_fi_junction_reads(int argc, char **argv);
    int cmd_fi_spanning_reads(int argc, char **argv);
    int cmd_fi_coalesce(int argc, char **argv);
    int cmd_fi_em(int argc, char **argv);
    int cmd_fi_ffpm(int argc, char **argv);
    int cmd_fi_frag_filter(int argc, char **argv);
    int cmd_fi_splice_info(int argc, char **argv);
    int cmd_fi_filter(int argc, char **argv);
    int cmd_fi_microh(int argc, char **argv);
    int cmd_fi_microh_dist(int argc, char **argv);
    int cmd_fi_cytoband(int argc, char **argv);
    int cmd_fi_gene_bed(int argc, char **argv);
    int cmd_fi_bed_tabix(int argc, char **argv);
    int cmd_fi_pfam(int argc, char **argv);
    int cmd_fi_seqsimilar(int argc, char **argv);
    int cmd_fi_gff3_to_bed(int argc, char **argv);
    int cmd_column_extractions(int argc, char **argv);
    int cmd_fi_retrieve_reads(int argc, char **argv);
    int cmd_fi_sam_to_bam(int argc, char **argv);
    int cmd_fi_index_bam(int argc, char **argv);
    int cmd_fi_report_json(int argc, char **argv);
    int cmd_fi_html(int argc, char **argv);
    int cmd_inspect(int argc, char **argv);
    int cmd_fi_tracks_json(int argc, char **argv);
    int cmd_run(int argc, char **argv);
    int cmd_dump_pairs(int argc, char **argv);
    int cmd_dump_bdb(int argc, char **argv);
} // namespace fsf