#include "formula/Functions.h"
#include "formula/Formula.h"

#include <QRegularExpression>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <functional>

// Batch 1 các hàm dựng sẵn (port _FUNCTIONS của formula.py). Các nhóm còn lại
// (tra cứu nâng cao, ngày/giờ, thống kê, lượng giác đầy đủ) bổ sung ở vòng sau.
namespace formula {
namespace {

[[noreturn]] void argErr(const char *fn) {
    throw FormulaError(QStringLiteral("%1: sai số đối số").arg(QLatin1String(fn)));
}

double sumv(const std::vector<double> &v) { return std::accumulate(v.begin(), v.end(), 0.0); }

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
