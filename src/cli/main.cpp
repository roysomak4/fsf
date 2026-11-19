// fsf main entry point for the command-line interface. Subcommands replace Perl stages of STAR fusion
#include <cstdio>
#include <malloc.h>
#include <cstring>

#include "cli/commands.hpp"
#include "core/log.hpp"
#include "version.hpp"

namespace
{

    struct Command
    {
        const char *name;
        int (*run)(int, char **);
        const char *help; // nullptr: hidden developer command
    };

    const Command kCommands[] = {
        {"run", fsf::cmd_run, "the whole pipeline: STAR, fusion calling, coding effect, FusionInspector"},
        {"map", fsf::cmd_map, "drop-in for STAR-Fusion.map_chimeric_reads_to_genes"},
        {"multimap", fsf::cmd_multimap, "drop-in for STAR-Fusion.handle_multimapping_reads"},
        {"predict", fsf::cmd_predict, "drop-in for STAR-Fusion.predict"},
        {"filter", fsf::cmd_filter, "drop-in for STAR-Fusion.filter (with FusionFilter's BLAST and promiscuity filters)"},
        {"ffpm", fsf::cmd_ffpm, "drop-in for incorporate_FFPM_into_final_report.pl"},
        {"splice-info", fsf::cmd_splice_info, "drop-in for append_breakpoint_junction_info.pl"},
        {"annotate", fsf::cmd_annotate, "drop-in for FusionAnnotator --annotate"},
        {"annot-filter", fsf::cmd_annot_filter, "drop-in for FusionFilter filter_by_annotation_rules.pl"},
        {"rt-filter", fsf::cmd_rt_filter, "drop-in for filter_likely_RT_artifacts.pl"},
        {"min-ffpm", fsf::cmd_min_ffpm, "drop-in for filter_by_min_FFPM.pl"},
        {"column-exclusions", fsf::cmd_column_exclusions, "drop-in for column_exclusions.pl"},
        {"coding-effect", fsf::cmd_coding_effect, "drop-in for FusionAnnotator fusion_to_coding_region_effect.pl"},
        {"evidence-reads", fsf::cmd_evidence_reads, "drop-in for get_FUSION_EVIDENCE_fastqs.pl"},
        {"fi-contigs", fsf::cmd_fi_contigs, "drop-in for FusionInspector fusion_pair_to_mini_genome_join.pl"},
        {"fi-star", fsf::cmd_fi_star, "drop-in for FusionInspector run_FI_STAR.pl (STAR on the fusion contigs)"},
        {"fi-dedup", fsf::cmd_fi_dedup, "drop-in for FusionInspector bam_mark_duplicates.py"},
        {"fi-junction-reads", fsf::cmd_fi_junction_reads, "drop-in for FusionInspector get_fusion_JUNCTION_reads_from_fusion_contig_bam.pl"},
        {"fi-spanning-reads", fsf::cmd_fi_spanning_reads, "drop-in for FusionInspector get_fusion_SPANNING_reads_from_bam.from_chim_summary.pl"},
        {"fi-coalesce", fsf::cmd_fi_coalesce, "drop-in for FusionInspector coalesce_junction_and_spanning_info.pl"},
        {"fi-em", fsf::cmd_fi_em, "drop-in for FusionInspector fusion_EM_runner.pl"},
        {"fi-ffpm", fsf::cmd_fi_ffpm, "drop-in for FusionInspector incorporate_FFPM_into_final_report.pl"},
        {"fi-frag-filter", fsf::cmd_fi_frag_filter, "drop-in for FusionInspector filter_fusions_by_frag_thresholds.pl"},
        {"fi-splice-info", fsf::cmd_fi_splice_info, "drop-in for FusionInspector append_breakpoint_junction_info_via_FI_contigs.pl"},
        {"fi-filter", fsf::cmd_fi_filter, "drop-in for FusionInspector's FusionFilter blast_and_promiscuity_filter.pl"},
        {"fi-microh", fsf::cmd_fi_microh, "drop-in for FusionInspector find_microhomologies_by_kmer_matches.pl"},
        {"fi-microh-dist", fsf::cmd_fi_microh_dist, "drop-in for FusionInspector append_microH_distance.py"},
        {"fi-cytoband", fsf::cmd_fi_cytoband, "drop-in for FusionInspector fasta_and_gtf_to_cytoband.pl"},
        {"fi-gene-bed", fsf::cmd_fi_gene_bed, "drop-in for FusionInspector gtf_gene_to_bed.pl"},
        {"fi-bed-tabix", fsf::cmd_fi_bed_tabix, "sort, bgzip and tabix-index a BED file (FusionInspector's gene track)"},
        {"fi-pfam", fsf::cmd_fi_pfam, "drop-in for FusionInspector get_pfam_domain_info.pl"},
        {"fi-seqsimilar", fsf::cmd_fi_seqsimilar, "drop-in for FusionInspector get_seq_similar_region_FI_coordinates.pl"},
        {"fi-gff3-to-bed", fsf::cmd_fi_gff3_to_bed, "drop-in for FusionInspector transcript_gff3_to_bed.pl"},
        {"column-extractions", fsf::cmd_column_extractions, "drop-in for FusionInspector column_extractions.pl"},
        {"fi-retrieve-reads", fsf::cmd_fi_retrieve_reads, "drop-in for FusionInspector retrieve_fusion_{junction,spanning}_reads_by_accession.pl"},
        {"fi-sam-to-bam", fsf::cmd_fi_sam_to_bam, "SAM records to a sorted, indexed BAM (samtools view -bT | sort; index)"},
        {"fi-index-bam", fsf::cmd_fi_index_bam, "samtools index"},
        {"fi-report-json", fsf::cmd_fi_report_json, "drop-in for FusionInspector create_fusion_inspector_igvjs.py"},
        {"fi-html", fsf::cmd_fi_html, "drop-in for igv-reports create_report --type fusion (FusionInspector's HTML)"},
        {"inspect", fsf::cmd_inspect, "drop-in for FusionInspector (contig-only inspect mode, as STAR-Fusion runs it)"},
        {"prep-lib", fsf::cmd_prep_lib, "one-time conversion of the genome library's Berkeley DB indexes"},
        {"_fi-tracks-json", fsf::cmd_fi_tracks_json, nullptr},
        {"_dump-pairs", fsf::cmd_dump_pairs, nullptr},
        {"_dump-bdb", fsf::cmd_dump_bdb, nullptr},
    };

    void print_version()
    {
        std::printf("fsf %s (%s) %s\n", FSF_VERSION, FSF_GIT_DESCRIBE, FSF_TARGET_NAME);
    }

    void print_usage(FILE *out)
    {
        std::fprintf(out,
                     "usage: fsf <command> [options]\n"
                     "       fsf --version\n\n"
                     "Commands (`fsf run` is the whole pipeline; `fsf <command> --help` lists its options;\n"
                     "the others are its stages, which fsf run and fsf inspect call):\n");
        int shown = 0;
        for (const auto &cmd : kCommands)
        {
            if (cmd.help)
            {
                std::fprintf(out, "  %-18s %s\n", cmd.name, cmd.help);
                shown++;
            }
        }
        if (!shown)
        {
            std::fprintf(out, "  (none yet)\n");
        }
    }
} // namespace

int main(int argc, char **argv)
{
    /*
    Large blocks (record and hit vectors, tables) always from mmap: freed memory goes back to the
    system, and growing a vector is an mremap rather than a copy. glibc's adaptive threshold
    would otherwise move them into arenas that keep freed memory.
    */
    mallopt(M_MAP_THRESHOLD, 1 << 20);
    fsf::log_init_from_env(); // an fsf step started by `fsf run` / `fsf inspect` logs to their files
    if (argc < 2)
    {
        print_usage(stderr);
        return 2;
    }
    const char *sub = argv[1];
    if (!std::strcmp(sub, "--version") || !std::strcmp(sub, "version"))
    {
        print_version();
        return 0;
    }
    if (!std::strcmp(sub, "-h") || !std::strcmp(sub, "--help") || !std::strcmp(sub, "help"))
    {
        print_usage(stdout);
        return 0;
    }
    for (const Command &c : kCommands)
        if (!std::strcmp(sub, c.name))
            return c.run(argc - 1, argv + 1);
    std::fprintf(stderr, "error: unknown command '%s' (see fsf --help)\n", sub);
    return 2;
}