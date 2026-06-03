#include "formula/Functions.h"
#include "formula/Formula.h"

#include <QRegularExpression>
#include <QDate>
#include <QDateTime>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <functional>
#include <set>

// Batch 1 các hàm dựng sẵn (port _FUNCTIONS của formula.py). Các nhóm còn lại
// (tra cứu nâng cao, ngày/giờ, thống kê, lượng giác đầy đủ) bổ sung ở vòng sau.
namespace formula {
namespace {

[[noreturn]] void argErr(const char *fn) {
    throw FormulaError(QStringLiteral("%1: sai số đối số").arg(QLatin1String(fn)));
}

double sumv(const std::vector<double> &v) { return std::accumulate(v.begin(), v.end(), 0.0); }

// flat 1 đối số (Range -> flat; else [v]). Port _crit_values.
std::vector<Value> critValues(const Value &a) {
    if (a.isRange()) return a.range.flat();
    return {a};
}

// --- ngày tháng: serial Excel (epoch 1899-12-30) <-> QDate ---
const QDate kExcelEpoch(1899, 12, 30);
double dateToSerial(const QDate &d) { return double(kExcelEpoch.daysTo(d)); }
QDate serialToDate(double n) {
    QDate d = kExcelEpoch.addDays(qint64(std::floor(n)));
    if (!d.isValid()) throw FormulaError(QStringLiteral("Ngày ngoài phạm vi"), ERR_NUM);
    return d;
}

// Regex wildcard kiểu Excel cho XLOOKUP match_mode=2.
QRegularExpression wildcardRe(const QString &pat) {
    QString out; int i = 0, n = pat.size();
    while (i < n) {
        QChar ch = pat[i];
        if (ch == '~' && i + 1 < n && (pat[i+1] == '*' || pat[i+1] == '?' || pat[i+1] == '~')) {
            out += QRegularExpression::escape(QString(pat[i+1])); i += 2; continue;
        }
        if (ch == '*') out += QStringLiteral(".*");
        else if (ch == '?') out += QStringLiteral(".");
        else out += QRegularExpression::escape(QString(ch));
        ++i;
    }
    return QRegularExpression(QStringLiteral("^") + out + QStringLiteral("$"),
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
}

// Tiêu chí kiểu Excel ("">5", "abc", 10) -> predicate(Value)->bool. Port _compile_criteria.
std::function<bool(const Value &)> compileCriteria(const Value &criteria) {
    if (criteria.type == Type::Bool) {
        bool target = criteria.boolean;
        return [target](const Value &v) { try { return toBool(v) == target; } catch (...) { return false; } };
    }
    if (criteria.type == Type::Number) {
        double target = criteria.num;
        return [target](const Value &v) { try { return toNumber(v) == target; } catch (...) { return false; } };
    }
    QString crit = toText(criteria);
    QString op;
    for (const QString &o : {QStringLiteral(">="), QStringLiteral("<="), QStringLiteral("<>"),
                             QStringLiteral(">"), QStringLiteral("<"), QStringLiteral("=")}) {
        if (crit.startsWith(o)) { op = o; crit = crit.mid(o.size()); break; }
    }
    bool hasNum = false;
    double critNum = crit.toDouble(&hasNum);
    if (hasNum && !std::isfinite(critNum)) hasNum = false;

    if (op == ">" || op == "<" || op == ">=" || op == "<=") {
        if (!hasNum) return [](const Value &) { return false; };
        QString o = op;
        return [o, critNum](const Value &v) {
            try {
                double x = toNumber(v);
                if (o == ">") return x > critNum;
                if (o == "<") return x < critNum;
                if (o == ">=") return x >= critNum;
                return x <= critNum;
            } catch (...) { return false; }
        };
    }
    bool negate = (op == "<>");
    if (hasNum) {
        return [critNum, negate](const Value &v) {
            bool eq = false;
            try { eq = toNumber(v) == critNum; } catch (...) { eq = false; }
            return negate ? !eq : eq;
        };
    }
    // So khớp chuỗi (hỗ trợ wildcard * ? ~).
    QString critLow = crit.toLower();
    bool wild = crit.contains('*') || crit.contains('?') || crit.contains('~');
    std::shared_ptr<QRegularExpression> rx;
    if (wild) {
        QString pat; int i = 0, n = crit.size();
        while (i < n) {
            QChar ch = crit[i];
            if (ch == '~' && i + 1 < n && (crit[i+1] == '*' || crit[i+1] == '?' || crit[i+1] == '~')) {
                pat += QRegularExpression::escape(QString(crit[i+1])); i += 2; continue;
            }
            if (ch == '*') pat += QStringLiteral(".*");
            else if (ch == '?') pat += QStringLiteral(".");
            else pat += QRegularExpression::escape(QString(ch));
            ++i;
        }
        rx = std::make_shared<QRegularExpression>(
            QStringLiteral("^") + pat + QStringLiteral("$"),
            QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
    }
    return [rx, critLow, negate](const Value &v) {
        QString s = toText(v);
        bool eq = rx ? rx->match(s).hasMatch() : (s.toLower() == critLow);
        return negate ? !eq : eq;
    };
}

// ----- registry build -----
QHash<QString, Fn> &fnMap() {
    static QHash<QString, Fn> m = [] {
        QHash<QString, Fn> r;
        auto need = [](const Args &a, int n, const char *fn) { if (int(a.size()) != n) argErr(fn); };

        // --- gộp / số học ---
        r["SUM"]     = [](const Args &a) { return Value::number(sumv(numbers(a))); };
        r["PRODUCT"] = [](const Args &a) { double p = 1; for (double n : numbers(a)) p *= n; return Value::number(p); };
        r["SUMSQ"]   = [](const Args &a) { double s = 0; for (double n : numbers(a)) s += n*n; return Value::number(s); };
        r["AVERAGE"] = [](const Args &a) { auto n = numbers(a); if (n.empty()) throw FormulaError(QStringLiteral("AVERAGE cần ít nhất một số")); return Value::number(sumv(n)/n.size()); };
        r["MIN"]     = [](const Args &a) { auto n = numbers(a); if (n.empty()) return Value::number(0); return Value::number(*std::min_element(n.begin(), n.end())); };
        r["MAX"]     = [](const Args &a) { auto n = numbers(a); if (n.empty()) return Value::number(0); return Value::number(*std::max_element(n.begin(), n.end())); };
        r["COUNT"]   = [](const Args &a) { return Value::number(double(numbers(a).size())); };
        r["COUNTA"]  = [](const Args &a) { int c = 0; for (auto &v : flatten(a)) if (!(v.type == Type::Empty || (v.type == Type::Text && v.text.isEmpty()))) ++c; return Value::number(c); };
        r["COUNTBLANK"] = [](const Args &a) { int c = 0; for (auto &v : flatten(a)) if (v.type == Type::Empty || (v.type == Type::Text && v.text.isEmpty())) ++c; return Value::number(c); };
        // Biến thể "A": số (kể cả chuỗi số) giữ giá trị, TRUE=1/FALSE=0, text không-số = 0; ô rỗng bỏ qua.
        auto valuesA = [](const Args &a) {
            std::vector<double> out;
            for (const Value &v : flatten(a)) {
                if (v.type == Type::Empty) continue;
                if (v.type == Type::Text && v.text.isEmpty()) continue; // coi như rỗng
                try { out.push_back(toNumber(v)); }      // số / chuỗi-số / bool
                catch (const FormulaError &) { out.push_back(0.0); } // text không-số -> 0
            }
            return out;
        };
        r["AVERAGEA"] = [valuesA](const Args &a) { auto n = valuesA(a); if (n.empty()) throw FormulaError(QStringLiteral("AVERAGEA chia 0"), ERR_DIV0); return Value::number(sumv(n)/n.size()); };
        r["MAXA"] = [valuesA](const Args &a) { auto n = valuesA(a); if (n.empty()) return Value::number(0); return Value::number(*std::max_element(n.begin(), n.end())); };
        r["MINA"] = [valuesA](const Args &a) { auto n = valuesA(a); if (n.empty()) return Value::number(0); return Value::number(*std::min_element(n.begin(), n.end())); };
        // PERCENTILE/QUARTILE: phân vị theo nội suy tuyến tính (phương pháp INC, gồm cả hai đầu).
        auto percentileInc = [](std::vector<double> v, double k) {
            std::sort(v.begin(), v.end());
            double rank = k * (v.size() - 1);
            int lo = int(std::floor(rank));
            double frac = rank - lo;
            if (lo + 1 < int(v.size())) return v[lo] + frac * (v[lo+1] - v[lo]);
            return v[lo];
        };
        r["PERCENTILE"] = [percentileInc](const Args &a) {
            if (a.size() != 2) argErr("PERCENTILE");
            auto v = numbers(std::vector<Value>{a[0]});
            if (v.empty()) throw FormulaError(QStringLiteral("PERCENTILE: vùng rỗng"), ERR_NUM);
            double k = toNumber(a[1]);
            if (k < 0 || k > 1) throw FormulaError(QStringLiteral("PERCENTILE: k trong [0,1]"), ERR_NUM);
            return Value::number(percentileInc(v, k));
        };
        r["QUARTILE"] = [percentileInc](const Args &a) {
            if (a.size() != 2) argErr("QUARTILE");
            auto v = numbers(std::vector<Value>{a[0]});
            if (v.empty()) throw FormulaError(QStringLiteral("QUARTILE: vùng rỗng"), ERR_NUM);
            int q = toInt(a[1]);
            if (q < 0 || q > 4) throw FormulaError(QStringLiteral("QUARTILE: phần tư 0..4"), ERR_NUM);
            return Value::number(percentileInc(v, q * 0.25));
        };
        // DEVSQ: tổng bình phương độ lệch so với trung bình. AVEDEV: độ lệch tuyệt đối trung bình.
        r["DEVSQ"] = [](const Args &a) {
            auto v = numbers(a);
            if (v.empty()) throw FormulaError(QStringLiteral("DEVSQ: vùng rỗng"), ERR_NUM);
            double m = sumv(v) / v.size(), s = 0;
            for (double x : v) s += (x - m) * (x - m);
            return Value::number(s);
        };
        r["AVEDEV"] = [](const Args &a) {
            auto v = numbers(a);
            if (v.empty()) throw FormulaError(QStringLiteral("AVEDEV: vùng rỗng"), ERR_NUM);
            double m = sumv(v) / v.size(), s = 0;
            for (double x : v) s += std::abs(x - m);
            return Value::number(s / v.size());
        };
        // SKEW: độ lệch (skewness) mẫu — cần >=3 giá trị. KURT: độ nhọn (kurtosis) mẫu — cần >=4.
        r["SKEW"] = [](const Args &a) {
            auto v = numbers(a);
            int n = int(v.size());
            if (n < 3) throw FormulaError(QStringLiteral("SKEW: cần ít nhất 3 số"), ERR_DIV0);
            double m = sumv(v) / n, ss = 0;
            for (double x : v) ss += (x - m) * (x - m);
            double s = std::sqrt(ss / (n - 1));
            if (s == 0) throw FormulaError(QStringLiteral("SKEW: độ lệch chuẩn 0"), ERR_DIV0);
            double sum3 = 0; for (double x : v) { double z = (x - m) / s; sum3 += z*z*z; }
            return Value::number(double(n) / ((n-1.0)*(n-2.0)) * sum3);
        };
        r["KURT"] = [](const Args &a) {
            auto v = numbers(a);
            int n = int(v.size());
            if (n < 4) throw FormulaError(QStringLiteral("KURT: cần ít nhất 4 số"), ERR_DIV0);
            double m = sumv(v) / n, ss = 0;
            for (double x : v) ss += (x - m) * (x - m);
            double s = std::sqrt(ss / (n - 1));
            if (s == 0) throw FormulaError(QStringLiteral("KURT: độ lệch chuẩn 0"), ERR_DIV0);
            double sum4 = 0; for (double x : v) { double z = (x - m) / s; sum4 += z*z*z*z; }
            double a1 = double(n)*(n+1.0) / ((n-1.0)*(n-2.0)*(n-3.0));
            double a2 = 3.0*(n-1.0)*(n-1.0) / ((n-2.0)*(n-3.0));
            return Value::number(a1 * sum4 - a2);
        };
        // TEXTBEFORE/TEXTAFTER(text, delim, [instance=1], [match_mode=0]): lấy phần trước/sau dấu phân cách.
        // instance âm -> đếm từ cuối; match_mode 1 -> không phân biệt hoa thường.
        auto matchPositions = [](const QString &s, const QString &d, Qt::CaseSensitivity cs) {
            std::vector<int> pos; int from = 0, idx;
            while ((idx = s.indexOf(d, from, cs)) >= 0) { pos.push_back(idx); from = idx + d.size(); }
            return pos;
        };
        r["TEXTBEFORE"] = [matchPositions](const Args &a) {
            if (a.size() < 2 || a.size() > 4) argErr("TEXTBEFORE");
            QString s = toText(a[0]), d = toText(a[1]);
            int inst = a.size() >= 3 ? toInt(a[2]) : 1;
            Qt::CaseSensitivity cs = (a.size() >= 4 && toInt(a[3]) == 1) ? Qt::CaseInsensitive : Qt::CaseSensitive;
            if (inst == 0) throw FormulaError(QStringLiteral("TEXTBEFORE: instance khác 0"), ERR_VALUE);
            if (d.isEmpty()) return Value::str(QString());
            auto pos = matchPositions(s, d, cs);
            int k = inst > 0 ? inst - 1 : int(pos.size()) + inst;
            if (k < 0 || k >= int(pos.size())) throw FormulaError(QStringLiteral("TEXTBEFORE: không tìm thấy"), ERR_NA);
            return Value::str(s.left(pos[k]));
        };
        r["TEXTAFTER"] = [matchPositions](const Args &a) {
            if (a.size() < 2 || a.size() > 4) argErr("TEXTAFTER");
            QString s = toText(a[0]), d = toText(a[1]);
            int inst = a.size() >= 3 ? toInt(a[2]) : 1;
            Qt::CaseSensitivity cs = (a.size() >= 4 && toInt(a[3]) == 1) ? Qt::CaseInsensitive : Qt::CaseSensitive;
            if (inst == 0) throw FormulaError(QStringLiteral("TEXTAFTER: instance khác 0"), ERR_VALUE);
            if (d.isEmpty()) return Value::str(s);
            auto pos = matchPositions(s, d, cs);
            int k = inst > 0 ? inst - 1 : int(pos.size()) + inst;
            if (k < 0 || k >= int(pos.size())) throw FormulaError(QStringLiteral("TEXTAFTER: không tìm thấy"), ERR_NA);
            return Value::str(s.mid(pos[k] + d.size()));
        };
        r["ABS"]   = [need](const Args &a) { need(a,1,"ABS"); return Value::number(std::abs(toNumber(a[0]))); };
        r["SIGN"]  = [need](const Args &a) { need(a,1,"SIGN"); double n = toNumber(a[0]); return Value::number((n>0)-(n<0)); };
        r["INT"]   = [need](const Args &a) { need(a,1,"INT"); return Value::number(std::floor(toNumber(a[0]))); };
        r["SQRT"]  = [need](const Args &a) { need(a,1,"SQRT"); double n = toNumber(a[0]); if (n<0) throw FormulaError(QStringLiteral("SQRT số âm"), ERR_NUM); return Value::number(std::sqrt(n)); };
        r["EXP"]   = [need](const Args &a) { need(a,1,"EXP"); return Value::number(std::exp(toNumber(a[0]))); };
        r["MOD"]   = [need](const Args &a) { need(a,2,"MOD"); double d = toNumber(a[1]); if (d==0) throw FormulaError(QStringLiteral("MOD chia 0"), ERR_DIV0); double res = std::fmod(toNumber(a[0]), d); if (res != 0 && ((res<0)!=(d<0))) res += d; return Value::number(res); };
        r["POWER"] = [need](const Args &a) { need(a,2,"POWER"); return Value::number(std::pow(toNumber(a[0]), toNumber(a[1]))); };
        r["SQRTPI"]= [need](const Args &a) { need(a,1,"SQRTPI"); return Value::number(std::sqrt(toNumber(a[0]) * M_PI)); };
        r["ROUND"] = [](const Args &a) { if (a.size()<1||a.size()>2) argErr("ROUND"); double n = toNumber(a[0]); int d = a.size()==2 ? toInt(a[1]) : 0; double f = std::pow(10,d); return Value::number(std::round(n*f)/f); };
        r["ROUNDUP"]   = [](const Args &a) { if (a.size()<1||a.size()>2) argErr("ROUNDUP"); double n = toNumber(a[0]); int d = a.size()==2 ? toInt(a[1]) : 0; double f = std::pow(10,d); return Value::number((n<0?std::floor(n*f):std::ceil(n*f))/f); };
        r["ROUNDDOWN"] = [](const Args &a) { if (a.size()<1||a.size()>2) argErr("ROUNDDOWN"); double n = toNumber(a[0]); int d = a.size()==2 ? toInt(a[1]) : 0; double f = std::pow(10,d); return Value::number((n<0?std::ceil(n*f):std::floor(n*f))/f); };
        r["TRUNC"] = [](const Args &a) { if (a.size()<1||a.size()>2) argErr("TRUNC"); double n = toNumber(a[0]); int d = a.size()==2 ? toInt(a[1]) : 0; double f = std::pow(10,d); return Value::number(std::trunc(n*f)/f); };
        r["CEILING"] = [need](const Args &a) { need(a,2,"CEILING"); double n = toNumber(a[0]), s = toNumber(a[1]); if (s==0) return Value::number(0); return Value::number(std::ceil(n/s)*s); };
        r["FLOOR"]   = [need](const Args &a) { need(a,2,"FLOOR"); double n = toNumber(a[0]), s = toNumber(a[1]); if (s==0) throw FormulaError(QStringLiteral("FLOOR bước 0"), ERR_DIV0); return Value::number(std::floor(n/s)*s); };
        r["LN"]    = [need](const Args &a) { need(a,1,"LN"); double n = toNumber(a[0]); if (n<=0) throw FormulaError(QStringLiteral("LN cần > 0"), ERR_NUM); return Value::number(std::log(n)); };
        r["LOG10"] = [need](const Args &a) { need(a,1,"LOG10"); double n = toNumber(a[0]); if (n<=0) throw FormulaError(QStringLiteral("LOG10 cần > 0"), ERR_NUM); return Value::number(std::log10(n)); };
        r["LOG"]   = [](const Args &a) { if (a.size()<1||a.size()>2) argErr("LOG"); double n = toNumber(a[0]); double base = a.size()==2 ? toNumber(a[1]) : 10.0; if (n<=0||base<=0||base==1) throw FormulaError(QStringLiteral("LOG đối số sai"), ERR_NUM); return Value::number(std::log(n)/std::log(base)); };
        r["SIN"] = [need](const Args &a) { need(a,1,"SIN"); return Value::number(std::sin(toNumber(a[0]))); };
        r["COS"] = [need](const Args &a) { need(a,1,"COS"); return Value::number(std::cos(toNumber(a[0]))); };
        r["TAN"] = [need](const Args &a) { need(a,1,"TAN"); return Value::number(std::tan(toNumber(a[0]))); };
        r["PI"]  = [](const Args &a) { if (!a.empty()) argErr("PI"); return Value::number(M_PI); };

        // --- logic (eager) ---
        r["AND"] = [](const Args &a) { for (auto &v : flatten(a)) if (!toBool(v)) return Value::boolv(false); return Value::boolv(true); };
        r["OR"]  = [](const Args &a) { for (auto &v : flatten(a)) if (toBool(v)) return Value::boolv(true); return Value::boolv(false); };
        r["XOR"] = [](const Args &a) { int t = 0; for (auto &v : flatten(a)) if (toBool(v)) ++t; return Value::boolv(t % 2 == 1); };
        r["NOT"] = [need](const Args &a) { need(a,1,"NOT"); return Value::boolv(!toBool(a[0])); };
        r["TRUE"]  = [](const Args &) { return Value::boolv(true); };
        r["FALSE"] = [](const Args &) { return Value::boolv(false); };

        // --- info ---
        r["ISNUMBER"] = [need](const Args &a) { need(a,1,"ISNUMBER"); return Value::boolv(a[0].type == Type::Number); };
        r["ISTEXT"]   = [need](const Args &a) { need(a,1,"ISTEXT"); return Value::boolv(a[0].type == Type::Text); };
        r["ISBLANK"]  = [need](const Args &a) { need(a,1,"ISBLANK"); return Value::boolv(a[0].type == Type::Empty || (a[0].type == Type::Text && a[0].text.isEmpty())); };
        r["ISLOGICAL"]= [need](const Args &a) { need(a,1,"ISLOGICAL"); return Value::boolv(a[0].type == Type::Bool); };
        r["N"] = [need](const Args &a) { need(a,1,"N"); try { return Value::number(toNumber(a[0])); } catch (...) { return Value::number(0); } };

        // --- chuỗi ---
        r["LEN"]   = [need](const Args &a) { need(a,1,"LEN"); return Value::number(toText(a[0]).size()); };
        r["UPPER"] = [need](const Args &a) { need(a,1,"UPPER"); return Value::str(toText(a[0]).toUpper()); };
        r["LOWER"] = [need](const Args &a) { need(a,1,"LOWER"); return Value::str(toText(a[0]).toLower()); };
        r["TRIM"]  = [need](const Args &a) { need(a,1,"TRIM"); return Value::str(toText(a[0]).simplified()); };
        r["LEFT"]  = [](const Args &a) { if (a.size()<1||a.size()>2) argErr("LEFT"); int n = a.size()==2 ? toInt(a[1]) : 1; return Value::str(toText(a[0]).left(std::max(0,n))); };
        r["RIGHT"] = [](const Args &a) { if (a.size()<1||a.size()>2) argErr("RIGHT"); int n = a.size()==2 ? toInt(a[1]) : 1; return Value::str(toText(a[0]).right(std::max(0,n))); };
        r["MID"]   = [need](const Args &a) { need(a,3,"MID"); int start = toInt(a[1]); int len = toInt(a[2]); if (start<1) throw FormulaError(QStringLiteral("MID vị trí < 1")); return Value::str(toText(a[0]).mid(start-1, std::max(0,len))); };
        r["REPT"]  = [need](const Args &a) { need(a,2,"REPT"); int n = toInt(a[1]); if (n<0) throw FormulaError(QStringLiteral("REPT số âm")); return Value::str(toText(a[0]).repeated(n)); };
        r["EXACT"] = [need](const Args &a) { need(a,2,"EXACT"); return Value::boolv(toText(a[0]) == toText(a[1])); };
        r["PROPER"]= [need](const Args &a) { need(a,1,"PROPER"); QString s = toText(a[0]); bool up = true; for (QChar &c : s) { if (c.isLetter()) { c = up ? c.toUpper() : c.toLower(); up = false; } else up = true; } return Value::str(s); };
        r["CONCAT"]      = [](const Args &a) { QString s; for (auto &v : flatten(a)) s += toText(v); return Value::str(s); };
        r["CONCATENATE"] = r["CONCAT"];
        r["SUBSTITUTE"]  = [](const Args &a) { if (a.size()<3||a.size()>4) argErr("SUBSTITUTE"); QString s = toText(a[0]), oldt = toText(a[1]), newt = toText(a[2]); if (oldt.isEmpty()) return Value::str(s); if (a.size()==3) { s.replace(oldt, newt); return Value::str(s); } int which = toInt(a[3]); int from = 0, cnt = 0; while (true) { int idx = s.indexOf(oldt, from); if (idx < 0) break; if (++cnt == which) { s.replace(idx, oldt.size(), newt); break; } from = idx + oldt.size(); } return Value::str(s); };
        r["FIND"]   = [](const Args &a) { if (a.size()<2||a.size()>3) argErr("FIND"); QString needle = toText(a[0]), hay = toText(a[1]); int start = a.size()==3 ? toInt(a[2]) : 1; int idx = hay.indexOf(needle, start-1, Qt::CaseSensitive); if (idx<0) throw FormulaError(QStringLiteral("FIND không thấy"), ERR_VALUE); return Value::number(idx+1); };
        r["SEARCH"] = [](const Args &a) { if (a.size()<2||a.size()>3) argErr("SEARCH"); QString needle = toText(a[0]), hay = toText(a[1]); int start = a.size()==3 ? toInt(a[2]) : 1; int idx = hay.indexOf(needle, start-1, Qt::CaseInsensitive); if (idx<0) throw FormulaError(QStringLiteral("SEARCH không thấy"), ERR_VALUE); return Value::number(idx+1); };
        r["TEXTJOIN"] = [](const Args &a) { if (a.size()<3) argErr("TEXTJOIN"); QString delim = toText(a[0]); bool skipEmpty = toBool(a[1]); QStringList parts; std::vector<Value> rest(a.begin()+2, a.end()); for (auto &v : flatten(rest)) { if (skipEmpty && (v.type == Type::Empty || (v.type==Type::Text && v.text.isEmpty()))) continue; parts << toText(v); } return Value::str(parts.join(delim)); };
        r["VALUE"] = [need](const Args &a) { need(a,1,"VALUE"); bool ok=false; double n = toText(a[0]).trimmed().toDouble(&ok); if (!ok) throw FormulaError(QStringLiteral("VALUE không phải số"), ERR_VALUE); return Value::number(n); };
        r["T"]     = [need](const Args &a) { need(a,1,"T"); return a[0].type == Type::Text ? a[0] : Value::str(QString()); };

        // --- điều kiện (COUNTIF/SUMIF/AVERAGEIF) ---
        r["COUNTIF"] = [need](const Args &a) { need(a,2,"COUNTIF"); auto pred = compileCriteria(a[1]); int c = 0; for (auto &v : flatten({a[0]})) if (pred(v)) ++c; return Value::number(c); };
        r["SUMIF"]   = [](const Args &a) { if (a.size()<2||a.size()>3) argErr("SUMIF"); auto pred = compileCriteria(a[1]); auto rng = flatten({a[0]}); std::vector<Value> sumRng = a.size()==3 ? flatten({a[2]}) : rng; double s = 0; for (size_t i=0;i<rng.size();++i) if (pred(rng[i]) && i<sumRng.size()) { try { s += toNumber(sumRng[i]); } catch (...) {} } return Value::number(s); };
        r["AVERAGEIF"] = [](const Args &a) { if (a.size()<2||a.size()>3) argErr("AVERAGEIF"); auto pred = compileCriteria(a[1]); auto rng = flatten({a[0]}); std::vector<Value> avgRng = a.size()==3 ? flatten({a[2]}) : rng; double s = 0; int c = 0; for (size_t i=0;i<rng.size();++i) if (pred(rng[i]) && i<avgRng.size()) { try { s += toNumber(avgRng[i]); ++c; } catch (...) {} } if (!c) throw FormulaError(QStringLiteral("AVERAGEIF không có số"), ERR_DIV0); return Value::number(s/c); };

        // --- tra cứu ---
        r["VLOOKUP"] = [](const Args &a) {
            if (a.size()<3||a.size()>4) argErr("VLOOKUP");
            if (!a[1].isRange()) throw FormulaError(QStringLiteral("VLOOKUP: đối số 2 phải là vùng"));
            const auto &rows = *a[1].range.rows;
            int width = a[1].range.width(), col = toInt(a[2]);
            bool approx = a.size()==4 ? toBool(a[3]) : true;
            if (col < 1 || col > width) throw FormulaError(QStringLiteral("VLOOKUP: số cột vượt phạm vi"));
            if (approx) {
                int best = -1;
                for (int i = 0; i < int(rows.size()); ++i) if (cmp(rows[i][0], a[0]) <= 0) best = i;
                if (best < 0) throw FormulaError(QStringLiteral("VLOOKUP: không tìm thấy"), ERR_NA);
                return rows[best][col-1];
            }
            for (const auto &row : rows) if (looseEqual(row[0], a[0])) return row[col-1];
            throw FormulaError(QStringLiteral("VLOOKUP: không tìm thấy"), ERR_NA);
        };
        r["HLOOKUP"] = [](const Args &a) {
            if (a.size()<3||a.size()>4) argErr("HLOOKUP");
            if (!a[1].isRange()) throw FormulaError(QStringLiteral("HLOOKUP: đối số 2 phải là vùng"));
            const auto &rows = *a[1].range.rows;
            int height = a[1].range.height(), ridx = toInt(a[2]);
            bool approx = a.size()==4 ? toBool(a[3]) : true;
            if (ridx < 1 || ridx > height) throw FormulaError(QStringLiteral("HLOOKUP: số hàng vượt phạm vi"));
            const auto &first = rows[0];
            if (approx) {
                int best = -1;
                for (int i = 0; i < int(first.size()); ++i) if (cmp(first[i], a[0]) <= 0) best = i;
                if (best < 0) throw FormulaError(QStringLiteral("HLOOKUP: không tìm thấy"), ERR_NA);
                return rows[ridx-1][best];
            }
            for (int i = 0; i < int(first.size()); ++i) if (looseEqual(first[i], a[0])) return rows[ridx-1][i];
            throw FormulaError(QStringLiteral("HLOOKUP: không tìm thấy"), ERR_NA);
        };
        r["INDEX"] = [](const Args &a) {
            if (a.size()<2||a.size()>3) argErr("INDEX");
            if (!a[0].isRange()) throw FormulaError(QStringLiteral("INDEX: đối số 1 phải là vùng"));
            int height = a[0].range.height(), width = a[0].range.width();
            int rr = toInt(a[1]), cc;
            if (a.size()==3) cc = toInt(a[2]);
            else if (height == 1) { cc = rr; rr = 1; }
            else cc = 1;
            if (!(rr>=1 && rr<=height && cc>=1 && cc<=width)) throw FormulaError(QStringLiteral("INDEX: chỉ số vượt phạm vi"), ERR_REF);
            return (*a[0].range.rows)[rr-1][cc-1];
        };
        r["MATCH"] = [](const Args &a) {
            if (a.size()<2||a.size()>3) argErr("MATCH");
            if (!a[1].isRange()) throw FormulaError(QStringLiteral("MATCH: đối số 2 phải là vùng"));
            auto vals = a[1].range.flat();
            int mtype = a.size()==3 ? toInt(a[2]) : 1;
            if (mtype == 0) {
                for (int i = 0; i < int(vals.size()); ++i) if (looseEqual(vals[i], a[0])) return Value::number(i+1);
                throw FormulaError(QStringLiteral("MATCH: không tìm thấy"), ERR_NA);
            }
            int best = -1;
            for (int i = 0; i < int(vals.size()); ++i) {
                int c = cmp(vals[i], a[0]);
                if ((mtype==1 && c<=0) || (mtype!=1 && c>=0)) best = i;
            }
            if (best < 0) throw FormulaError(QStringLiteral("MATCH: không tìm thấy"), ERR_NA);
            return Value::number(best+1);
        };
        r["LOOKUP"] = [](const Args &a) {
            if (a.size()<2||a.size()>3) argErr("LOOKUP");
            auto sv = critValues(a[1]);
            auto rv = a.size()==3 ? critValues(a[2]) : sv;
            int best = -1;
            for (int i = 0; i < int(sv.size()); ++i) if (cmp(sv[i], a[0]) <= 0) best = i;
            if (best < 0 || best >= int(rv.size())) throw FormulaError(QStringLiteral("LOOKUP: không tìm thấy"), ERR_NA);
            return rv[best];
        };
        r["CHOOSE"] = [](const Args &a) {
            if (a.size() < 2) argErr("CHOOSE");
            int idx = toInt(a[0]);
            if (idx < 1 || idx > int(a.size())-1) throw FormulaError(QStringLiteral("CHOOSE: index vượt phạm vi"), ERR_VALUE);
            return a[idx];
        };
        r["XLOOKUP"] = [](const Args &a) {
            if (a.size()<3||a.size()>6) argErr("XLOOKUP");
            auto look = critValues(a[1]);
            auto ret = critValues(a[2]);
            int matchMode = a.size()>=5 ? toInt(a[4]) : 0;
            int searchMode = a.size()>=6 ? toInt(a[5]) : 1;
            int n = int(look.size());
            std::vector<int> order;
            if (searchMode >= 0) for (int i=0;i<n;++i) order.push_back(i);
            else for (int i=n-1;i>=0;--i) order.push_back(i);
            int found = -1;
            if (matchMode == 0) {
                for (int i : order) if (looseEqual(look[i], a[0])) { found = i; break; }
            } else if (matchMode == 2) {
                QRegularExpression rx = wildcardRe(toText(a[0]));
                for (int i : order) if (rx.match(toText(look[i])).hasMatch()) { found = i; break; }
            } else if (matchMode == -1 || matchMode == 1) {
                bool wantSmaller = (matchMode == -1);
                int best = -1;
                for (int i : order) {
                    int c = cmp(look[i], a[0]);
                    if (c == 0) { found = i; break; }
                    if ((wantSmaller && c < 0) || (!wantSmaller && c > 0)) {
                        if (best < 0 || (wantSmaller ? cmp(look[i], look[best]) > 0 : cmp(look[i], look[best]) < 0)) best = i;
                    }
                }
                if (found < 0) found = best;
            } else throw FormulaError(QStringLiteral("XLOOKUP: match_mode không hỗ trợ"), ERR_VALUE);
            if (found < 0) {
                if (a.size() >= 4) return a[3];
                throw FormulaError(QStringLiteral("XLOOKUP: không tìm thấy"), ERR_NA);
            }
            if (found >= int(ret.size())) throw FormulaError(QStringLiteral("XLOOKUP: vùng trả về không khớp"), ERR_VALUE);
            return ret[found];
        };
        r["ROWS"]    = [need](const Args &a) { need(a,1,"ROWS"); return Value::number(a[0].isRange() ? a[0].range.height() : 1); };
        r["COLUMNS"] = [need](const Args &a) { need(a,1,"COLUMNS"); return Value::number(a[0].isRange() ? a[0].range.width() : 1); };

        // --- ngày / giờ (serial Excel) ---
        r["TODAY"] = [](const Args &a) { if (!a.empty()) argErr("TODAY"); return Value::number(dateToSerial(QDate::currentDate())); };
        r["NOW"]   = [](const Args &a) { if (!a.empty()) argErr("NOW"); QDateTime n = QDateTime::currentDateTime(); double s = dateToSerial(n.date()); QTime t = n.time(); return Value::number(s + (t.hour()*3600 + t.minute()*60 + t.second())/86400.0); };
        r["DATE"]  = [need](const Args &a) { need(a,3,"DATE"); QDate d(toInt(a[0]), toInt(a[1]), toInt(a[2])); if (!d.isValid()) throw FormulaError(QStringLiteral("DATE không hợp lệ")); return Value::number(dateToSerial(d)); };
        r["YEAR"]  = [need](const Args &a) { need(a,1,"YEAR"); return Value::number(serialToDate(toNumber(a[0])).year()); };
        r["MONTH"] = [need](const Args &a) { need(a,1,"MONTH"); return Value::number(serialToDate(toNumber(a[0])).month()); };
        r["DAY"]   = [need](const Args &a) { need(a,1,"DAY"); return Value::number(serialToDate(toNumber(a[0])).day()); };
        r["HOUR"]   = [need](const Args &a) { need(a,1,"HOUR"); double s = toNumber(a[0]); int sec = int(std::round((s - std::floor(s)) * 86400)); return Value::number((sec/3600) % 24); };
        r["MINUTE"] = [need](const Args &a) { need(a,1,"MINUTE"); double s = toNumber(a[0]); int sec = int(std::round((s - std::floor(s)) * 86400)); return Value::number((sec/60) % 60); };
        r["SECOND"] = [need](const Args &a) { need(a,1,"SECOND"); double s = toNumber(a[0]); int sec = int(std::round((s - std::floor(s)) * 86400)); return Value::number(sec % 60); };
        r["DAYS"]   = [need](const Args &a) { need(a,2,"DAYS"); return Value::number(serialToDate(toNumber(a[1])).daysTo(serialToDate(toNumber(a[0])))); };
        r["WEEKDAY"] = [](const Args &a) { if (a.size()<1||a.size()>2) argErr("WEEKDAY"); int iso = serialToDate(toNumber(a[0])).dayOfWeek(); int t = a.size()==2 ? toInt(a[1]) : 1; if (t==1) return Value::number(iso % 7 + 1); if (t==2) return Value::number(iso); if (t==3) return Value::number(iso - 1); throw FormulaError(QStringLiteral("WEEKDAY: type không hỗ trợ")); };
        r["DATEDIF"] = [need](const Args &a) { need(a,3,"DATEDIF"); QDate s = serialToDate(toNumber(a[0])), e = serialToDate(toNumber(a[1])); QString u = toText(a[2]).toUpper(); if (u=="D") return Value::number(s.daysTo(e)); if (u=="Y") return Value::number(e.year()-s.year() - ((e.month()<s.month()||(e.month()==s.month()&&e.day()<s.day()))?1:0)); if (u=="M") return Value::number((e.year()-s.year())*12 + (e.month()-s.month()) - (e.day()<s.day()?1:0)); throw FormulaError(QStringLiteral("DATEDIF: unit phải D/M/Y")); };
        // EDATE(start, months): cộng số tháng, tự dồn ngày cuối tháng (31/1 +1 -> 29/2).
        r["EDATE"] = [need](const Args &a) { need(a,2,"EDATE"); QDate d = serialToDate(toNumber(a[0])).addMonths(toInt(a[1])); return Value::number(dateToSerial(d)); };
        // EOMONTH(start, months): ngày cuối cùng của tháng sau khi cộng months.
        r["EOMONTH"] = [need](const Args &a) { need(a,2,"EOMONTH"); QDate d = serialToDate(toNumber(a[0])).addMonths(toInt(a[1])); QDate eom(d.year(), d.month(), d.daysInMonth()); return Value::number(dateToSerial(eom)); };
        // Gom ngày nghỉ (đối số tùy chọn, có thể là vùng) -> tập serial (đã làm tròn).
        auto holidaySet = [](const Args &a, int from) {
            std::set<qint64> hs;
            for (int i = from; i < int(a.size()); ++i)
                for (const Value &v : critValues(a[i]))
                    if (v.type == Type::Number) hs.insert(qint64(std::floor(v.num)));
            return hs;
        };
        auto isWorkday = [](const QDate &d, const std::set<qint64> &hs) {
            int dow = d.dayOfWeek(); // 6=T7, 7=CN
            return dow != 6 && dow != 7 && !hs.count(qint64(dateToSerial(d)));
        };
        // WORKDAY(start, days, [holidays]): ngày sau `days` ngày làm việc (bỏ T7/CN + nghỉ).
        r["WORKDAY"] = [need,holidaySet,isWorkday](const Args &a) {
            if (a.size() < 2) argErr("WORKDAY");
            QDate d = serialToDate(toNumber(a[0])); int days = toInt(a[1]);
            auto hs = holidaySet(a, 2); int step = days >= 0 ? 1 : -1; int left = std::abs(days);
            while (left > 0) { d = d.addDays(step); if (isWorkday(d, hs)) --left; }
            return Value::number(dateToSerial(d));
        };
        // NETWORKDAYS(start, end, [holidays]): đếm ngày làm việc trong khoảng (gồm 2 đầu).
        r["NETWORKDAYS"] = [need,holidaySet,isWorkday](const Args &a) {
            if (a.size() < 2) argErr("NETWORKDAYS");
            QDate s = serialToDate(toNumber(a[0])), e = serialToDate(toNumber(a[1]));
            int sign = 1; if (s > e) { std::swap(s, e); sign = -1; }
            auto hs = holidaySet(a, 2); int cnt = 0;
            for (QDate d = s; d <= e; d = d.addDays(1)) if (isWorkday(d, hs)) ++cnt;
            return Value::number(double(sign * cnt));
        };
        // ISOWEEKNUM(serial): tuần ISO 8601 (tuần bắt đầu Thứ Hai, tuần 1 chứa Thứ Năm đầu năm).
        r["ISOWEEKNUM"] = [need](const Args &a) { need(a,1,"ISOWEEKNUM"); return Value::number(serialToDate(toNumber(a[0])).weekNumber()); };
        // WEEKNUM(serial, [type]): tuần 1 chứa ngày 1/1. type 1 (mặc định) tuần bắt đầu CN, type 2 bắt đầu T2.
        r["WEEKNUM"] = [](const Args &a) {
            if (a.size() < 1 || a.size() > 2) argErr("WEEKNUM");
            QDate d = serialToDate(toNumber(a[0]));
            int type = a.size() == 2 ? toInt(a[1]) : 1;
            QDate jan1(d.year(), 1, 1);
            int startDow = (type == 2) ? (jan1.dayOfWeek() - 1) : (jan1.dayOfWeek() % 7); // 0-index từ ngày đầu tuần
            int doy = int(jan1.daysTo(d)); // số ngày từ 1/1 (0-based)
            return Value::number((doy + startDow) / 7 + 1);
        };
        // TIME(giờ,phút,giây) -> phần lẻ của ngày. T(v) -> chuỗi nếu là text. N(v) -> số.
        r["TIME"] = [need](const Args &a) { need(a,3,"TIME"); double s = toNumber(a[0])*3600 + toNumber(a[1])*60 + toNumber(a[2]); return Value::number(std::fmod(s/86400.0 + 1.0, 1.0)); };
        r["T"]    = [need](const Args &a) { need(a,1,"T"); return a[0].type == Type::Text ? a[0] : Value::str(QString()); };
        r["N"]    = [need](const Args &a) { need(a,1,"N"); const Value &v = a[0]; if (v.type == Type::Number) return v; if (v.type == Type::Bool) return Value::number(v.boolean ? 1 : 0); return Value::number(0); };
        // FIXED/DOLLAR: định dạng số -> chuỗi (dấu phẩy hàng nghìn). BASE: đổi cơ số.
        auto withCommas = [](QString s) {
            int dot = s.indexOf('.'); int end = (dot < 0) ? s.size() : dot;
            int start = (s.size() && s[0] == '-') ? 1 : 0;
            for (int i = end - 3; i > start; i -= 3) s.insert(i, ',');
            return s;
        };
        r["FIXED"] = [withCommas](const Args &a) { if (a.size()<1||a.size()>3) argErr("FIXED"); double n = toNumber(a[0]); int dec = a.size()>=2 ? toInt(a[1]) : 2; bool noComma = a.size()>=3 && toBool(a[2]); QString s = QString::number(n, 'f', qMax(0, dec)); return Value::str(noComma ? s : withCommas(s)); };
        r["DOLLAR"] = [withCommas](const Args &a) { if (a.size()<1||a.size()>2) argErr("DOLLAR"); double n = toNumber(a[0]); int dec = a.size()>=2 ? toInt(a[1]) : 2; QString s = QString::number(std::abs(n), 'f', qMax(0, dec)); return Value::str((n<0?"-$":"$") + withCommas(s)); };
        r["BASE"] = [](const Args &a) { if (a.size()<2||a.size()>3) argErr("BASE"); long long n = (long long)toNumber(a[0]); int radix = toInt(a[1]); if (radix<2||radix>36) throw FormulaError(QStringLiteral("BASE: cơ số 2-36")); int minLen = a.size()>=3 ? toInt(a[2]) : 0; QString s = QString::number(n, radix).toUpper(); while (s.size() < minLen) s.prepend('0'); return Value::str(s); };
        // Đổi hệ cơ số: DECIMAL (chuỗi->số), DEC2BIN/DEC2HEX (số->chuỗi), BIN2DEC/HEX2DEC (chuỗi->số).
        auto pad = [](QString s, const Args &a, int idx) { int p = (int)a.size() > idx ? toInt(a[idx]) : 0; while (s.size() < p) s.prepend('0'); return s; };
        r["DECIMAL"] = [need](const Args &a) { need(a,2,"DECIMAL"); int radix = toInt(a[1]); if (radix<2||radix>36) throw FormulaError(QStringLiteral("DECIMAL: cơ số 2-36")); bool ok=false; long long n = toText(a[0]).toLongLong(&ok, radix); if (!ok) throw FormulaError(QStringLiteral("DECIMAL: chuỗi không hợp lệ")); return Value::number(n); };
        r["DEC2BIN"] = [pad](const Args &a) { if (a.size()<1||a.size()>2) argErr("DEC2BIN"); return Value::str(pad(QString::number((long long)toNumber(a[0]), 2), a, 1)); };
        r["DEC2HEX"] = [pad](const Args &a) { if (a.size()<1||a.size()>2) argErr("DEC2HEX"); return Value::str(pad(QString::number((long long)toNumber(a[0]), 16).toUpper(), a, 1)); };
        r["BIN2DEC"] = [need](const Args &a) { need(a,1,"BIN2DEC"); bool ok=false; long long n = toText(a[0]).toLongLong(&ok, 2); if (!ok) throw FormulaError(QStringLiteral("BIN2DEC: chuỗi nhị phân không hợp lệ")); return Value::number(n); };
        r["HEX2DEC"] = [need](const Args &a) { need(a,1,"HEX2DEC"); bool ok=false; long long n = toText(a[0]).toLongLong(&ok, 16); if (!ok) throw FormulaError(QStringLiteral("HEX2DEC: chuỗi thập lục không hợp lệ")); return Value::number(n); };

        // --- thống kê ---
        r["MEDIAN"] = [](const Args &a) { auto n = numbers(a); if (n.empty()) throw FormulaError(QStringLiteral("MEDIAN cần ít nhất một số")); std::sort(n.begin(), n.end()); size_t m = n.size(); return Value::number(m%2 ? n[m/2] : (n[m/2-1]+n[m/2])/2.0); };
        r["MODE"]   = [](const Args &a) { auto n = numbers(a); if (n.empty()) throw FormulaError(QStringLiteral("MODE cần ít nhất một số")); std::sort(n.begin(), n.end()); double best = n[0]; int bestc = 0, cur = 1; for (size_t i=1;i<=n.size();++i) { if (i<n.size() && n[i]==n[i-1]) ++cur; else { if (cur>bestc) { bestc=cur; best=n[i-1]; } cur=1; } } return Value::number(best); };
        r["STDEV"]  = [](const Args &a) { auto n = numbers(a); if (n.size()<2) throw FormulaError(QStringLiteral("STDEV cần ít nhất hai số")); double m = sumv(n)/n.size(), s = 0; for (double x : n) s += (x-m)*(x-m); return Value::number(std::sqrt(s/(n.size()-1))); };
        r["STDEVP"] = [](const Args &a) { auto n = numbers(a); if (n.empty()) throw FormulaError(QStringLiteral("STDEVP cần ít nhất một số")); double m = sumv(n)/n.size(), s = 0; for (double x : n) s += (x-m)*(x-m); return Value::number(std::sqrt(s/n.size())); };
        r["VAR"]    = [](const Args &a) { auto n = numbers(a); if (n.size()<2) throw FormulaError(QStringLiteral("VAR cần ít nhất hai số")); double m = sumv(n)/n.size(), s = 0; for (double x : n) s += (x-m)*(x-m); return Value::number(s/(n.size()-1)); };
        r["VARP"]   = [](const Args &a) { auto n = numbers(a); if (n.empty()) throw FormulaError(QStringLiteral("VARP cần ít nhất một số")); double m = sumv(n)/n.size(), s = 0; for (double x : n) s += (x-m)*(x-m); return Value::number(s/n.size()); };
        r["LARGE"]  = [need](const Args &a) { need(a,2,"LARGE"); auto n = numbers({a[0]}); int k = toInt(a[1]); if (k<1||k>int(n.size())) throw FormulaError(QStringLiteral("LARGE: k vượt phạm vi")); std::sort(n.begin(), n.end(), std::greater<double>()); return Value::number(n[k-1]); };
        r["SMALL"]  = [need](const Args &a) { need(a,2,"SMALL"); auto n = numbers({a[0]}); int k = toInt(a[1]); if (k<1||k>int(n.size())) throw FormulaError(QStringLiteral("SMALL: k vượt phạm vi")); std::sort(n.begin(), n.end()); return Value::number(n[k-1]); };
        r["RANK"]   = [](const Args &a) { if (a.size()<2||a.size()>3) argErr("RANK"); double x = toNumber(a[0]); auto n = numbers({a[1]}); int order = a.size()==3 ? toInt(a[2]) : 0; if (order) std::sort(n.begin(), n.end()); else std::sort(n.begin(), n.end(), std::greater<double>()); for (int i=0;i<int(n.size());++i) if (n[i]==x) return Value::number(i+1); throw FormulaError(QStringLiteral("RANK: không tìm thấy")); };
        r["GEOMEAN"]= [](const Args &a) { auto n = numbers(a); if (n.empty()) throw FormulaError(QStringLiteral("GEOMEAN cần số")); double p = 1; for (double x : n) { if (x<=0) throw FormulaError(QStringLiteral("GEOMEAN cần số dương"), ERR_NUM); p *= x; } return Value::number(std::pow(p, 1.0/n.size())); };
        r["HARMEAN"]= [](const Args &a) { auto n = numbers(a); if (n.empty()) throw FormulaError(QStringLiteral("HARMEAN cần số")); double s = 0; for (double x : n) { if (x==0) throw FormulaError(QStringLiteral("HARMEAN cần khác 0"), ERR_NUM); s += 1.0/x; } return Value::number(n.size()/s); };

        // --- đa điều kiện (COUNTIFS/SUMIFS/AVERAGEIFS/MAXIFS/MINIFS) ---
        // pairs bắt đầu từ chỉ số `base` (cặp range,criteria). Trả các index hàng thỏa.
        auto matchRows = [](const Args &a, int base, const char *fn) {
            std::vector<std::vector<Value>> ranges;
            std::vector<std::function<bool(const Value &)>> preds;
            for (size_t i = base; i + 1 < a.size(); i += 2) {
                ranges.push_back(flatten({a[i]}));
                preds.push_back(compileCriteria(a[i+1]));
            }
            size_t n = ranges.empty() ? 0 : ranges[0].size();
            for (auto &rg : ranges) if (rg.size() != n) throw FormulaError(QStringLiteral("%1: vùng khác kích thước").arg(QLatin1String(fn)));
            std::vector<int> hit;
            for (size_t i = 0; i < n; ++i) {
                bool all = true;
                for (size_t k = 0; k < preds.size(); ++k) if (!preds[k](ranges[k][i])) { all = false; break; }
                if (all) hit.push_back(int(i));
            }
            return hit;
        };
        r["COUNTIFS"] = [matchRows](const Args &a) { if (a.size()<2||a.size()%2!=0) argErr("COUNTIFS"); return Value::number(matchRows(a, 0, "COUNTIFS").size()); };
        r["SUMIFS"] = [matchRows](const Args &a) { if (a.size()<3||a.size()%2==0) argErr("SUMIFS"); auto sv = flatten({a[0]}); auto hit = matchRows(a, 1, "SUMIFS"); double s = 0; for (int i : hit) if (i < int(sv.size())) { try { s += toNumber(sv[i]); } catch (...) {} } return Value::number(s); };
        r["AVERAGEIFS"] = [matchRows](const Args &a) { if (a.size()<3||a.size()%2==0) argErr("AVERAGEIFS"); auto sv = flatten({a[0]}); auto hit = matchRows(a, 1, "AVERAGEIFS"); double s = 0; int c = 0; for (int i : hit) if (i < int(sv.size())) { try { s += toNumber(sv[i]); ++c; } catch (...) {} } if (!c) throw FormulaError(QStringLiteral("AVERAGEIFS không có số"), ERR_DIV0); return Value::number(s/c); };
        r["MAXIFS"] = [matchRows](const Args &a) { if (a.size()<3||a.size()%2==0) argErr("MAXIFS"); auto sv = flatten({a[0]}); auto hit = matchRows(a, 1, "MAXIFS"); double best = 0; bool any = false; for (int i : hit) if (i < int(sv.size())) { try { double v = toNumber(sv[i]); if (!any || v > best) { best = v; any = true; } } catch (...) {} } return Value::number(any ? best : 0); };
        r["MINIFS"] = [matchRows](const Args &a) { if (a.size()<3||a.size()%2==0) argErr("MINIFS"); auto sv = flatten({a[0]}); auto hit = matchRows(a, 1, "MINIFS"); double best = 0; bool any = false; for (int i : hit) if (i < int(sv.size())) { try { double v = toNumber(sv[i]); if (!any || v < best) { best = v; any = true; } } catch (...) {} } return Value::number(any ? best : 0); };
        r["SUMPRODUCT"] = [](const Args &a) { if (a.empty()) return Value::number(0); std::vector<std::vector<Value>> arr; for (auto &x : a) arr.push_back(critValues(x)); size_t n = arr[0].size(); for (auto &x : arr) n = std::min(n, x.size()); double tot = 0; for (size_t i=0;i<n;++i) { double p = 1; for (auto &x : arr) p *= toNumber(x[i]); tot += p; } return Value::number(tot); };

        // --- math mở rộng ---
        auto gcd2 = [](long long a, long long b) { a = std::llabs(a); b = std::llabs(b); while (b) { long long t = a % b; a = b; b = t; } return a; };
        r["GCD"] = [gcd2](const Args &a) { long long g = 0; for (double x : numbers(a)) g = gcd2(g, (long long)x); return Value::number(double(g)); };
        r["LCM"] = [gcd2](const Args &a) { long long l = 1; for (double x : numbers(a)) { long long v = (long long)x; if (v==0) return Value::number(0); l = l / gcd2(l, v) * v; } return Value::number(double(l)); };
        r["FACT"] = [need](const Args &a) { need(a,1,"FACT"); int n = toInt(a[0]); if (n<0) throw FormulaError(QStringLiteral("FACT cần >= 0"), ERR_NUM); double f = 1; for (int i=2;i<=n;++i) f *= i; return Value::number(f); };
        r["COMBIN"] = [need](const Args &a) { need(a,2,"COMBIN"); int n = toInt(a[0]), k = toInt(a[1]); if (k<0||n<0||k>n) throw FormulaError(QStringLiteral("COMBIN đối số sai"), ERR_NUM); double c = 1; for (int i=0;i<k;++i) c = c*(n-i)/(i+1); return Value::number(std::round(c)); };
        r["PERMUT"] = [need](const Args &a) { need(a,2,"PERMUT"); int n = toInt(a[0]), k = toInt(a[1]); if (k<0||n<0||k>n) throw FormulaError(QStringLiteral("PERMUT đối số sai"), ERR_NUM); double p = 1; for (int i=0;i<k;++i) p *= (n-i); return Value::number(p); };
        r["MROUND"] = [need](const Args &a) { need(a,2,"MROUND"); double n = toNumber(a[0]), m = toNumber(a[1]); if (m==0) return Value::number(0); return Value::number(std::round(n/m)*m); };
        r["QUOTIENT"] = [need](const Args &a) { need(a,2,"QUOTIENT"); double d = toNumber(a[1]); if (d==0) throw FormulaError(QStringLiteral("QUOTIENT chia 0"), ERR_DIV0); return Value::number(std::trunc(toNumber(a[0])/d)); };
        r["EVEN"] = [need](const Args &a) { need(a,1,"EVEN"); double n = toNumber(a[0]); double c = std::ceil(std::abs(n)/2.0)*2; return Value::number(n<0 ? -c : c); };
        r["ODD"]  = [need](const Args &a) { need(a,1,"ODD"); double n = toNumber(a[0]); double m = std::abs(n); double c = std::floor((m-1)/2.0)*2+1; if (c<m) c += 2; return Value::number(n<0 ? -c : c); };
        r["ATAN2"] = [need](const Args &a) { need(a,2,"ATAN2"); return Value::number(std::atan2(toNumber(a[1]), toNumber(a[0]))); };
        r["DEGREES"] = [need](const Args &a) { need(a,1,"DEGREES"); return Value::number(toNumber(a[0]) * 180.0 / M_PI); };
        r["RADIANS"] = [need](const Args &a) { need(a,1,"RADIANS"); return Value::number(toNumber(a[0]) * M_PI / 180.0); };
        r["ASIN"] = [need](const Args &a) { need(a,1,"ASIN"); double x = toNumber(a[0]); if (x<-1||x>1) throw FormulaError(QStringLiteral("ASIN miền [-1,1]"), ERR_NUM); return Value::number(std::asin(x)); };
        r["ACOS"] = [need](const Args &a) { need(a,1,"ACOS"); double x = toNumber(a[0]); if (x<-1||x>1) throw FormulaError(QStringLiteral("ACOS miền [-1,1]"), ERR_NUM); return Value::number(std::acos(x)); };
        r["ATAN"] = [need](const Args &a) { need(a,1,"ATAN"); return Value::number(std::atan(toNumber(a[0]))); };
        r["SINH"] = [need](const Args &a) { need(a,1,"SINH"); return Value::number(std::sinh(toNumber(a[0]))); };
        r["COSH"] = [need](const Args &a) { need(a,1,"COSH"); return Value::number(std::cosh(toNumber(a[0]))); };
        r["TANH"] = [need](const Args &a) { need(a,1,"TANH"); return Value::number(std::tanh(toNumber(a[0]))); };
        // Lượng giác nghịch đảo: SEC=1/cos, CSC=1/sin, COT=1/tan (báo lỗi khi chia 0).
        r["SEC"] = [need](const Args &a) { need(a,1,"SEC"); double c = std::cos(toNumber(a[0])); if (c==0) throw FormulaError(QStringLiteral("SEC chia 0"), ERR_DIV0); return Value::number(1.0/c); };
        r["CSC"] = [need](const Args &a) { need(a,1,"CSC"); double s = std::sin(toNumber(a[0])); if (s==0) throw FormulaError(QStringLiteral("CSC chia 0"), ERR_DIV0); return Value::number(1.0/s); };
        r["COT"] = [need](const Args &a) { need(a,1,"COT"); double t = std::tan(toNumber(a[0])); if (t==0) throw FormulaError(QStringLiteral("COT chia 0"), ERR_DIV0); return Value::number(1.0/t); };
        // Hyperbolic nghịch đảo: SECH=1/cosh, CSCH=1/sinh, COTH=1/tanh.
        r["SECH"] = [need](const Args &a) { need(a,1,"SECH"); return Value::number(1.0/std::cosh(toNumber(a[0]))); };
        r["CSCH"] = [need](const Args &a) { need(a,1,"CSCH"); double s = std::sinh(toNumber(a[0])); if (s==0) throw FormulaError(QStringLiteral("CSCH chia 0"), ERR_DIV0); return Value::number(1.0/s); };
        r["COTH"] = [need](const Args &a) { need(a,1,"COTH"); double t = std::tanh(toNumber(a[0])); if (t==0) throw FormulaError(QStringLiteral("COTH chia 0"), ERR_DIV0); return Value::number(1.0/t); };
        // Hàm hyperbolic ngược: ASINH (mọi x), ACOSH (x>=1), ATANH (-1<x<1).
        r["ASINH"] = [need](const Args &a) { need(a,1,"ASINH"); return Value::number(std::asinh(toNumber(a[0]))); };
        r["ACOSH"] = [need](const Args &a) { need(a,1,"ACOSH"); double x = toNumber(a[0]); if (x<1) throw FormulaError(QStringLiteral("ACOSH miền x>=1"), ERR_NUM); return Value::number(std::acosh(x)); };
        r["ATANH"] = [need](const Args &a) { need(a,1,"ATANH"); double x = toNumber(a[0]); if (x<=-1||x>=1) throw FormulaError(QStringLiteral("ATANH miền (-1,1)"), ERR_NUM); return Value::number(std::atanh(x)); };
        // Bitwise (kỹ thuật): số nguyên không âm < 2^48. BITAND/BITOR/BITXOR.
        auto bitArg = [](const Value &v, const char *fn) -> qint64 {
            double d = toNumber(v);
            if (d < 0 || d != std::floor(d) || d >= 281474976710656.0) // 2^48
                throw FormulaError(QString::fromUtf8(fn) + QStringLiteral(": cần số nguyên không âm < 2^48"), ERR_NUM);
            return qint64(d);
        };
        r["BITAND"] = [need,bitArg](const Args &a) { need(a,2,"BITAND"); return Value::number(double(bitArg(a[0],"BITAND") & bitArg(a[1],"BITAND"))); };
        r["BITOR"]  = [need,bitArg](const Args &a) { need(a,2,"BITOR");  return Value::number(double(bitArg(a[0],"BITOR")  | bitArg(a[1],"BITOR"))); };
        r["BITXOR"] = [need,bitArg](const Args &a) { need(a,2,"BITXOR"); return Value::number(double(bitArg(a[0],"BITXOR") ^ bitArg(a[1],"BITXOR"))); };
        // BITLSHIFT/BITRSHIFT: dịch trái/phải `shift` bit (shift âm -> dịch ngược). |shift| <= 53.
        r["BITLSHIFT"] = [need,bitArg](const Args &a) { need(a,2,"BITLSHIFT"); qint64 n = bitArg(a[0],"BITLSHIFT"); int s = toInt(a[1]); if (s<-53||s>53) throw FormulaError(QStringLiteral("BITLSHIFT: |shift| <= 53"), ERR_NUM); return Value::number(s>=0 ? double(n << s) : double(n >> (-s))); };
        r["BITRSHIFT"] = [need,bitArg](const Args &a) { need(a,2,"BITRSHIFT"); qint64 n = bitArg(a[0],"BITRSHIFT"); int s = toInt(a[1]); if (s<-53||s>53) throw FormulaError(QStringLiteral("BITRSHIFT: |shift| <= 53"), ERR_NUM); return Value::number(s>=0 ? double(n >> s) : double(n << (-s))); };
        // ROMAN(số): đổi số 0..3999 sang chữ số La Mã (dạng cổ điển). 0 -> rỗng.
        r["ROMAN"] = [](const Args &a) {
            if (a.size() < 1 || a.size() > 2) argErr("ROMAN");
            int n = toInt(a[0]);
            if (n < 0 || n > 3999) throw FormulaError(QStringLiteral("ROMAN: số trong [0,3999]"), ERR_VALUE);
            static const int vals[] = {1000,900,500,400,100,90,50,40,10,9,5,4,1};
            static const char *syms[] = {"M","CM","D","CD","C","XC","L","XL","X","IX","V","IV","I"};
            QString out;
            for (int i = 0; i < 13; ++i) while (n >= vals[i]) { out += QLatin1String(syms[i]); n -= vals[i]; }
            return Value::str(out);
        };
        // ARABIC(chữ La Mã): đổi ngược về số nguyên (hỗ trợ cú pháp trừ IV, IX...).
        r["ARABIC"] = [need](const Args &a) {
            need(a,1,"ARABIC");
            QString s = toText(a[0]).trimmed().toUpper();
            int sign = 1; if (s.startsWith('-')) { sign = -1; s = s.mid(1); }
            auto rv = [](QChar c) -> int { switch (c.toLatin1()) { case 'I': return 1; case 'V': return 5; case 'X': return 10; case 'L': return 50; case 'C': return 100; case 'D': return 500; case 'M': return 1000; default: return -1; } };
            int total = 0, prev = 0;
            for (int i = s.size() - 1; i >= 0; --i) {
                int v = rv(s[i]);
                if (v < 0) throw FormulaError(QStringLiteral("ARABIC: chữ La Mã không hợp lệ"), ERR_VALUE);
                if (v < prev) total -= v; else { total += v; prev = v; }
            }
            return Value::number(double(sign * total));
        };

        // DELTA(a,[b]): 1 nếu a==b (delta Kronecker), ngược lại 0. b mặc định 0.
        r["DELTA"] = [](const Args &a) { if (a.size()<1||a.size()>2) argErr("DELTA"); double x = toNumber(a[0]); double y = a.size()==2 ? toNumber(a[1]) : 0.0; return Value::number(x==y ? 1 : 0); };
        // GESTEP(num,[step]): 1 nếu num >= step, ngược lại 0. step mặc định 0.
        r["GESTEP"] = [](const Args &a) { if (a.size()<1||a.size()>2) argErr("GESTEP"); double x = toNumber(a[0]); double s = a.size()==2 ? toNumber(a[1]) : 0.0; return Value::number(x>=s ? 1 : 0); };
        // YEARFRAC(start, end, [basis]): phần năm giữa hai ngày theo quy ước đếm ngày.
        // basis 0=US 30/360 (mặc định), 1=thực/thực, 2=thực/360, 3=thực/365, 4=Âu 30/360.
        r["YEARFRAC"] = [](const Args &a) {
            if (a.size() < 2 || a.size() > 3) argErr("YEARFRAC");
            QDate d1 = serialToDate(toNumber(a[0])), d2 = serialToDate(toNumber(a[1]));
            int basis = a.size() == 3 ? toInt(a[2]) : 0;
            if (basis < 0 || basis > 4) throw FormulaError(QStringLiteral("YEARFRAC: basis 0..4"), ERR_NUM);
            if (d1 > d2) std::swap(d1, d2);
            auto days360 = [](QDate s, QDate e, bool eu) {
                int sd = s.day(), ed = e.day();
                if (eu) { if (sd == 31) sd = 30; if (ed == 31) ed = 30; }
                else { if (sd == 31) sd = 30; if (ed == 31 && sd == 30) ed = 30; }
                return (e.year()-s.year())*360 + (e.month()-s.month())*30 + (ed - sd);
            };
            double diff = double(d1.daysTo(d2));
            switch (basis) {
                case 0: return Value::number(days360(d1, d2, false) / 360.0);
                case 4: return Value::number(days360(d1, d2, true) / 360.0);
                case 2: return Value::number(diff / 360.0);
                case 3: return Value::number(diff / 365.0);
                default: { // basis 1: thực/thực — chia cho độ dài năm trung bình trong khoảng
                    if (d1.year() == d2.year())
                        return Value::number(diff / (QDate::isLeapYear(d1.year()) ? 366.0 : 365.0));
                    double totalDays = double(QDate(d1.year(),1,1).daysTo(QDate(d2.year(),12,31)) + 1);
                    double avg = totalDays / (d2.year() - d1.year() + 1);
                    return Value::number(diff / avg);
                }
            }
        };

        // --- text/info mở rộng ---
        r["CHAR"]    = [need](const Args &a) { need(a,1,"CHAR"); int c = toInt(a[0]); if (c<1||c>255) throw FormulaError(QStringLiteral("CHAR mã 1..255"), ERR_VALUE); return Value::str(QString(QChar(c))); };
        r["UNICHAR"] = [need](const Args &a) { need(a,1,"UNICHAR"); int c = toInt(a[0]); if (c<1) throw FormulaError(QStringLiteral("UNICHAR mã > 0"), ERR_VALUE); return Value::str(QString(QChar(c))); };
        r["CODE"]    = [need](const Args &a) { need(a,1,"CODE"); QString s = toText(a[0]); if (s.isEmpty()) throw FormulaError(QStringLiteral("CODE chuỗi rỗng"), ERR_VALUE); return Value::number(s.at(0).unicode()); };
        r["UNICODE"] = r["CODE"];
        r["CLEAN"]   = [need](const Args &a) { need(a,1,"CLEAN"); QString s = toText(a[0]), o; for (QChar c : s) if (c.unicode() >= 32) o += c; return Value::str(o); };
        r["REPLACE"] = [need](const Args &a) { need(a,4,"REPLACE"); QString s = toText(a[0]); int start = toInt(a[1]), num = toInt(a[2]); QString nw = toText(a[3]); if (start<1) throw FormulaError(QStringLiteral("REPLACE vị trí < 1")); s.replace(start-1, std::max(0,num), nw); return Value::str(s); };
        r["ISEVEN"]  = [need](const Args &a) { need(a,1,"ISEVEN"); return Value::boolv(((long long)std::trunc(toNumber(a[0]))) % 2 == 0); };
        r["ISODD"]   = [need](const Args &a) { need(a,1,"ISODD"); return Value::boolv(((long long)std::trunc(toNumber(a[0]))) % 2 != 0); };
        r["ISNONTEXT"] = [need](const Args &a) { need(a,1,"ISNONTEXT"); return Value::boolv(a[0].type != Type::Text); };
        r["NA"] = [](const Args &) -> Value { throw FormulaError(QStringLiteral("NA"), ERR_NA); };

        return r;
    }();
    return m;
}

QHash<QString, LazyFn> &lazyMap() {
    static QHash<QString, LazyFn> m = [] {
        QHash<QString, LazyFn> r;
        r["IF"] = [](const std::vector<Thunk> &t) -> Value {
            if (t.size() < 2 || t.size() > 3) argErr("IF");
            bool cond = toBool(t[0]());
            if (cond) return t[1]();
            return t.size() == 3 ? t[2]() : Value::boolv(false);
        };
        r["IFERROR"] = [](const std::vector<Thunk> &t) -> Value {
            if (t.size() != 2) argErr("IFERROR");
            try { return t[0](); } catch (const FormulaError &) { return t[1](); }
        };
        r["IFNA"] = [](const std::vector<Thunk> &t) -> Value {
            if (t.size() != 2) argErr("IFNA");
            try { return t[0](); }
            catch (const FormulaError &e) { if (e.etype() == ERR_NA) return t[1](); throw; }
        };
        r["IFS"] = [](const std::vector<Thunk> &t) -> Value {
            if (t.empty() || t.size() % 2 != 0) argErr("IFS");
            for (size_t i = 0; i + 1 < t.size(); i += 2)
                if (toBool(t[i]())) return t[i+1]();
            throw FormulaError(QStringLiteral("IFS không điều kiện nào đúng"), ERR_NA);
        };
        r["ISERROR"] = [](const std::vector<Thunk> &t) -> Value {
            if (t.size() != 1) argErr("ISERROR");
            try { t[0](); return Value::boolv(false); } catch (const FormulaError &) { return Value::boolv(true); }
        };
        r["ISERR"] = [](const std::vector<Thunk> &t) -> Value {
            if (t.size() != 1) argErr("ISERR");
            try { t[0](); return Value::boolv(false); }
            catch (const FormulaError &e) { return Value::boolv(e.etype() != ERR_NA); }
        };
        r["ISNA"] = [](const std::vector<Thunk> &t) -> Value {
            if (t.size() != 1) argErr("ISNA");
            try { t[0](); return Value::boolv(false); }
            catch (const FormulaError &e) { return Value::boolv(e.etype() == ERR_NA); }
        };
        r["SWITCH"] = [](const std::vector<Thunk> &t) -> Value {
            if (t.size() < 3) argErr("SWITCH");
            Value subject = t[0]();
            size_t i = 1;
            for (; i + 1 < t.size(); i += 2)
                if (looseEqual(subject, t[i]())) return t[i+1]();
            if (i < t.size()) return t[i](); // default lẻ cuối
            throw FormulaError(QStringLiteral("SWITCH không khớp"), ERR_NA);
        };
        return r;
    }();
    return m;
}

} // anonymous namespace

const Fn *lookupFunction(const QString &upperName) {
    auto &m = fnMap();
    auto it = m.constFind(upperName);
    return it == m.constEnd() ? nullptr : &it.value();
}

const LazyFn *lookupLazy(const QString &upperName) {
    auto &m = lazyMap();
    auto it = m.constFind(upperName);
    return it == m.constEnd() ? nullptr : &it.value();
}

} // namespace formula
