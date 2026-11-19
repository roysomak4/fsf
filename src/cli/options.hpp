/* Command-line options: "--name value", "--name=value", and "-x value", "-xvalue" or bundled
   "-dJ file" for one-letter names. Long names are lower case with words joined by '-'
   (--genome-lib-dir). Exact names only: an unknown option is an error, and an old STAR-Fusion
   spelling (--genome_lib_dir, --CPU) gets a hint naming the new one. Words that aren't options
   are kept in leftover().
*/
#pragma once
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "core/common.hpp"

namespace fsf
{

    class Options
    {
    public:
        // `names` lists a long name and/or one letter, e.g. "chimeric-junction|J".
        void str(const char *names, std::string *out) { add(names, Kind::Str, out); }
        void integer(const char *names, long *out) { add(names, Kind::Int, out); }
        void flag(const char *names, bool *out) { add(names, Kind::Flag, out); }
        void number(const char *names, double *out) { add(names, Kind::Num, out); }

        // Word parse() didn't use (not options), in order.
        const std::vector<std::string> &leftover() const { return leftover_; }

        // Returns false (after printing "error: ..." to stderr) for an unknown option, or a missing or
        // malformed value.
        bool parse(int argc, char **argv)
        {
            for (int i = 1; i < argc; i++)
            {
                const char *a = argv[i];
                if (a[0] != '-' || a[1] == 0)
                {
                    leftover_.push_back(a);
                    continue;
                }
                if (a[1] == '-')
                {
                    if (a[2] == 0)
                    {
                        for (int k = i + 1; k < argc; k++)
                            leftover_.push_back(argv[k]);
                        break;
                    } // "--" ends options
                    std::string name = a + 2, value;
                    bool has_value = false;
                    size_t eq = name.find('=');
                    if (eq != std::string::npos)
                    {
                        value = name.substr(eq + 1);
                        name.resize(eq);
                        has_value = true;
                    }
                    Spec *s = find(name);
                    if (!s)
                        return unknown("--" + name);
                    if (s->kind == Kind::Flag)
                    {
                        *static_cast<bool *>(s->out) = true;
                        continue;
                    }
                    if (!has_value)
                    {
                        if (i + 1 >= argc)
                            return missing(name);
                        value = argv[++i];
                    }
                    if (!store(*s, name, value))
                        return false;
                }
                else
                {
                    // Bundled one-letter options: "-dJ file" or "-Jfile".
                    for (const char *p = a + 1; *p; p++)
                    {
                        Spec *s = find(std::string(1, *p));
                        if (!s)
                            return unknown(std::string("-") + *p);
                        if (s->kind == Kind::Flag)
                        {
                            *static_cast<bool *>(s->out) = true;
                            continue;
                        }
                        std::string value;
                        if (p[1])
                            value = p + 1;
                        else if (i + 1 < argc)
                            value = argv[++i];
                        else
                            return missing(std::string(1, *p));
                        if (!store(*s, std::string(1, *p), value))
                            return false;
                        break;
                    }
                }
            }
            return true;
        }

    private:
        enum class Kind
        {
            Str,
            Int,
            Flag,
            Num
        };
        struct Spec
        {
            std::vector<std::string> names;
            Kind kind;
            void *out;
        };
        std::vector<Spec> specs_;
        std::vector<std::string> leftover_;

        void add(const char *names, Kind k, void *out)
        {
            Spec s{{}, k, out};
            std::string all = names;
            size_t p = 0;
            while (true)
            {
                size_t q = all.find('|', p);
                s.names.push_back(all.substr(p, q == std::string::npos ? std::string::npos : q - p));
                if (q == std::string::npos)
                    break;
                p = q + 1;
            }
            specs_.push_back(s);
        }
        Spec *find(const std::string &name)
        {
            for (Spec &s : specs_)
                for (const std::string &n : s.names)
                    if (n == name)
                        return &s;
            return nullptr;
        }
        static bool missing(const std::string &name)
        {
            std::fprintf(stderr, "error: option %s%s needs a value\n", name.size() == 1 ? "-" : "--", name.c_str());
            return false;
        }
        // The name without case, '-' or '_': how an old spelling is matched to its new name.
        static std::string loose(const std::string &n)
        {
            std::string k;
            for (char c : n)
                if (c != '-' && c != '_')
                    k += char(std::tolower(static_cast<unsigned char>(c)));
            return k;
        }
        bool unknown(const std::string &opt) const
        {
            const std::string want = loose(opt);
            for (const Spec &s : specs_)
                for (const std::string &n : s.names)
                    if (n.size() > 1 && loose(n) == want)
                    {
                        std::fprintf(stderr, "error: unknown option %s (use --%s)\n", opt.c_str(), n.c_str());
                        return false;
                    }
            std::fprintf(stderr, "error: unknown option %s\n", opt.c_str());
            return false;
        }
        static bool store(Spec &s, const std::string &name, const std::string &value)
        {
            if (s.kind == Kind::Str)
            {
                *static_cast<std::string *>(s.out) = value;
                return true;
            }
            if (s.kind == Kind::Num)
            {
                char *end = nullptr;
                double d = std::strtod(value.c_str(), &end);
                if (value.empty() || *end)
                {
                    std::fprintf(stderr, "error: option %s%s needs a number, not \"%s\"\n", name.size() == 1 ? "-" : "--", name.c_str(), value.c_str());
                    return false;
                }
                *static_cast<double *>(s.out) = d;
                return true;
            }
            int64_t v;
            if (!parse_i64(value, v))
            {
                std::fprintf(stderr, "error: option %s%s needs a whole number, not \"%s\"\n", name.size() == 1 ? "-" : "--", name.c_str(), value.c_str());
                return false;
            }
            *static_cast<long *>(s.out) = long(v);
            return true;
        }
    };

} // namespace fsf