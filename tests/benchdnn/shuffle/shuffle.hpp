/*******************************************************************************
* Copyright 2018-2020 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#ifndef SHUFFLE_HPP
#define SHUFFLE_HPP

#include <assert.h>
#include <limits.h>
#include <stdint.h>

#include <iostream>

#include "common.hpp"
#include "dnn_types.hpp"
#include "dnnl_common.hpp"
#include "dnnl_debug.hpp"
#include "dnnl_memory.hpp"
#include "perf_report.hpp"

namespace shuffle {

struct settings_t {
    settings_t() = default;

    // ctor to save certain fields from resetting
    settings_t(const char *perf_template) : settings_t() {
        this->perf_template = perf_template;
    }

    dims_t dims;

    std::vector<dir_t> dir {FWD_D};
    std::vector<dnnl_data_type_t> dt {dnnl_f32};
    std::vector<std::string> tag {tag::abx};
    std::vector<int64_t> group {1};
    std::vector<int> axis {1};
    bool allow_unimpl = false;

    const char *perf_template_csv
            = "perf,%engine%,%dir%,%dt%,%tag%,%group%,%axis%,%DESC%,%-time%,%"
              "0time%";
    const char *perf_template_def = "perf,%engine%,%prb%,%-time%,%0time%";
    const char *perf_template = perf_template_def;

    void reset() { *this = settings_t(perf_template); }
};

struct prb_t {
    prb_t(const dims_t &dims, dir_t dir, dnnl_data_type_t dt,
            const std::string &tag, int axis, int64_t group)
        : dims(dims)
        , dir(dir)
        , dt(dt)
        , tag(tag)
        , axis(axis)
        , group(group)
        , ndims((int)dims.size()) {}
    ~prb_t() {}

    dims_t dims;
    dir_t dir;
    dnnl_data_type_t dt;
    std::string tag;
    int axis;
    int64_t group;
    int ndims;
};
std::ostream &operator<<(std::ostream &s, const prb_t &p);

struct perf_report_t : public base_perf_report_t {
    using base_perf_report_t::base_perf_report_t;

    void report(const prb_t *p, const res_t *r, const char *prb_str) {
        p_ = p;
        base_report(r, prb_str);
    }

    virtual void dump_desc(std::ostream &s) const override { s << p_->dims; }

    virtual void dump_desc_csv(std::ostream &s) const override {
        s << p_->dims;
    }

    virtual const int *axis() const override { return &p_->axis; }
    virtual const int64_t *group() const override { return &p_->group; }
    virtual const dir_t *dir() const override { return &p_->dir; }
    virtual const dnnl_data_type_t *dt() const override { return &p_->dt; }
    virtual const std::string *tag() const override { return &p_->tag; }

private:
    const prb_t *p_ = NULL;
};

inline size_t data_off(const prb_t *p, int64_t mb, int64_t c, int64_t d,
        int64_t h, int64_t w) {
    const auto &dims = p->dims;
    return (((mb * dims[1] + c) * dims[2] + d) * dims[3] + h) * dims[4] + w;
}

void compute_shuffle(const prb_t *p, const dnn_mem_t &src, dnn_mem_t &dst);
int doit(const prb_t *p, res_t *res);
int bench(int argc, char **argv);
} // namespace shuffle

#endif
