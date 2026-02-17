// Copyright 2015 MakerBot Industries

#ifndef INCLUDE_MBCOREUTILS_JCPP_H_
#define INCLUDE_MBCOREUTILS_JCPP_H_

#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/writer.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include <string>
#include <sstream>
#include <map>
#include <memory>
#include <iostream>
#include <stdexcept>


namespace CallBacker {
    struct payload;
    struct Discovery {
        virtual ~Discovery() {}
	virtual void operator()(const payload&){}
    };

    struct payload {
        virtual ~payload() {}
        virtual void Clear() {}
        private:
        friend std::ostream& operator<<(std::ostream&, const payload&);
        virtual std::ostream& operator<<(std::ostream& o) const { return o; }
    };
    inline std::ostream& operator<<(std::ostream& o, const payload& p) {
        return p.operator<<(o);
    }
}  //  namespace CallBacker

/*
Note: In it's primitive form, this technique is not designed to 
handle objects that multilple keys with the same name, and maintains
the objects in alphabetical order, not the order of appearance.
These objects could be modified and / or enhanced to address both of 
these issues, if needed. 

Arrays are sorted by the order of appearence, objects are sorted 
alphabetically.
 */


namespace JCpp {
    inline const std::string quoted(const std::string& what) {
        if (what.empty()) return "\"\"";
        std::stringstream ss;
        ss << "\"" << what << "\"";
        std::string ret(ss.str());
        return ret;
    }

    inline const std::string jtrim(const std::string what) {
        std::string tmp(what);
        const std::string tbd("\": \t\n\r");
        while (!tmp.empty()) {
            size_t found(tmp.find_first_of(tbd));
            if (found == 0)
                tmp.erase(0, 1);
            else
                break;
        }
        while (!tmp.empty()) {
            size_t found(tmp.find_last_of(tbd));
            if (found == tmp.size()-1)
                tmp.erase(tmp.size()-1, 1);
                    else
                        break;
        }
        size_t found(tmp.find("//"));
        if (found != std::string::npos)
            tmp.resize(found);
        while (!tmp.empty()) {
            size_t found(tmp.find_last_of(tbd));
            if (found == tmp.size()-1)
                tmp.erase(tmp.size()-1, 1);
                    else
                        break;
        }
        return tmp;
    }


    struct PrimitiveBase {
        explicit PrimitiveBase(PrimitiveBase* _parent)
            : parent(_parent) {}
        virtual ~PrimitiveBase() {}
        virtual operator std::string& () = 0;
        virtual PrimitiveBase& operator()(std::string What) = 0;
        virtual PrimitiveBase* generate
            (PrimitiveBase* creator, const std::string& _name) = 0;
        virtual PrimitiveBase* generate
            (PrimitiveBase* creator, const std::string& _name,
		const Json::ValueType& _type)
	{ 
                static_cast<void>(_type);
		return generate(creator, _name); 
	} 
        virtual PrimitiveBase& operator()
            (std::string name, std::string value,
             const Json::ValueType& _type = Json::stringValue,
             int _depth = 1) {
                static_cast<void>(name);
                static_cast<void>(value);
                static_cast<void>(_type);
                static_cast<void>(_depth);
                return *this;
        }
        virtual void Create(Json::Value& json,
                const int _depth = 0, int order = -1) = 0;
        virtual void operator=(std::string) {}
        virtual bool propagate(const std::string& txt) {
            static_cast<void>(txt);
            return false;
        }
        virtual operator bool () {return false;}
        virtual std::ostream& print(std::ostream& o,
            const Json::ValueType& _type) const {
                static_cast<void>(_type);
                return o;
        }
        virtual std::ostream& printxml(std::ostream& o,
            const Json::ValueType& _type) const {
                static_cast<void>(_type);
                return o;
        }
        PrimitiveBase& GetParent() const {
            assert(parent);
            return *parent;
        }
        PrimitiveBase* Parent() const {
            return parent;
        }
        PrimitiveBase& GetTrunk() {
            if (parent) return parent->GetTrunk();
            return *this;
        }
        virtual int Depth(int newdepth=-1) const {
            static_cast<void>(newdepth);
            return -1;
        }
        virtual PrimitiveBase* Local(std::string What) {
            static_cast<void>(What);
            return nullptr;
        }
        virtual int32_t LocalLong(std::string What, int32_t deflt = 0) {
            static_cast<void>(What);
            return deflt;
        }
        virtual float LocalFloat(std::string What, float deflt = 0) {
            static_cast<void>(What);
            return deflt;
        }
        virtual const std::string LocalString
            (std::string What, const std::string& deflt="") {
                static_cast<void>(What);
                return deflt;
        }
        virtual void CallBack(const std::string) {}
        virtual void CallBack(CallBacker::payload&) {}
        virtual void CallBack(CallBacker::payload&, CallBacker::payload&) {}
        virtual void TickBack() {}
        virtual bool GetRunningState() {return true;}

        protected:
        PrimitiveBase* parent;

        private:
        friend std::ostream& operator<<(std::ostream&, const PrimitiveBase&);
        virtual std::ostream& operator<<(std::ostream& o) const  {return o;}
    };

    struct PrimitiveKey : protected std::string {
        PrimitiveKey(const PrimitiveKey& k)
            : std::string(k), type(k.type), order(k.order) {}
        PrimitiveKey(const std::string& k, Json::ValueType _type)
            : std::string(k), type(_type), order(-1) {}
        PrimitiveKey(const std::string& k, Json::ValueType _type, int _order)
            : std::string(k), type(_type), order(_order) {}
        virtual bool operator==(const Json::ValueType& _t) const {
            return _t == type;
        }
        operator Json::ValueType () const {return type;}
        virtual std::ostream& print(std::ostream& o,
            const Json::ValueType& _type) const {
                if (_type != Json::arrayValue)
                    o << quoted(c_str());
                return o;
        }
        std::string name() const {return c_str();}
        int Order() const {return order;}
        virtual bool operator < (const PrimitiveKey& k) const {
            //  This operation defines the rules for sorting arrays or objects
            //  These rules may be modified and / or enhanced to provide
            //  different functionality
            //  DO NOT USE THE MUTABLES IN COMPARISONS!
            const std::string& me(*this);
            if ( ( k.order > -1) && ( order > -1 ) )
                return order < k.order;
            return me < k;
        }
        //  void operator = (const Json::ValueType _type)
        //    { type=_type;} // SHOULD THIS BE SETTABLE??

        private:
        friend std::ostream& operator<<(std::ostream&, const PrimitiveKey&);

        protected:
        const Json::ValueType type;  // SHOULD THIS BE MUTABLE??
        const int order;
    };

    template <typename PNK>
        struct PrimitiveNode : std::map<PNK, PrimitiveBase*>,
            public PrimitiveBase {
            typedef PNK KeyType;
            typedef std::map<PNK, PrimitiveBase*> ContainerType;
            explicit PrimitiveNode(PrimitiveBase* _parent)
                : PrimitiveBase(_parent), depth(0) {}

            virtual void Create(Json::Value& json,
                const int _depth = 0, int order = -1) {
                    Generate<PNK>(json, _depth, order);
            }

            template <typename CT>
                void Generate(Json::Value& json,
                    const int _depth = 0, int order = -1) {
                    depth = _depth;
                    if ( (json.type() != Json::objectValue)
                            && (json.type() != Json::arrayValue) ) {
                        std::stringstream ssv;
                        ssv << json;
                        value = jtrim(ssv.str());
                    }
                    if (order == -1)
                        if (json.begin() != json.end()) {
                            Json::ValueIterator it(json.begin());
                            Json::Value& val(*it);
                            if (val.type() == Json::arrayValue) order = 0;
                        }
                    for ( Json::ValueIterator it = json.begin();
                        it != json.end(); it++ ) {
                        Json::Value& val(*it);
                        std::stringstream ssname;
                        ssname << it.key();
                        const std::string name(jtrim(ssname.str()));
                        const PNK key(name, val.type(), order);
                        if (order>-1) order++;
                        if (this->find(key) != this->end())
                            throw std::runtime_error(
                                "More than one exists->" + name);
                        PrimitiveNode<PNK>* n(static_cast<PrimitiveNode<PNK>*>(
                            generate(this, name, val.type())));
                        PrimitiveNode<PNK>& newnode(*n);
                        PrimitiveNode<PNK>& me(*this);
                        me[key]=n;
                        newnode.Create(val, depth + 1, order);
                    }
                }

            virtual PrimitiveBase& operator()(std::string What) {
                const PNK key(What, Json::stringValue, -1);
                typename ContainerType::iterator found(this->find(key));
                if (found == this->end())
                    throw std::runtime_error("Cannot find " + What);
                return *found->second;
            }

            virtual const PNK& RequireKey(std::string What) {
                const PNK key(What, Json::stringValue, -1);
                typename ContainerType::iterator found(this->find(key));
                if (found == this->end())
                    throw std::runtime_error("Cannot find " + What);
                return found->first;
            }

            virtual PrimitiveBase* Local(std::string What) {
                const PNK key(What, Json::stringValue, -1);
                typename PrimitiveNode<PNK>::iterator found(this->find(key));
                if (found == this->end()) return nullptr;
                return found->second;
            }

            virtual const std::string LocalString(std::string What,
                const std::string& deflt = "") {
                static_cast<void>(deflt);
                const PNK key(What, Json::stringValue, -1);
                typename PrimitiveNode<PNK>::iterator found(this->find(key));
                if (found == this->end()) return "";
                return *found->second;
            }

            virtual float LocalFloat(std::string What, float deflt = 0) {
                static_cast<void>(deflt);
                const PNK key(What, Json::stringValue, -1);
                typename PrimitiveNode<PNK>::iterator
                    found(this->find(key));
                if (found == this->end()) return 0;
                std::string& v(*found->second);
                if (v.empty()) return 0;
                return atof(v.c_str());
            }

            virtual int32_t LocalLong(std::string What, int32_t deflt = 0) {
                static_cast<void>(deflt);
                const PNK key(What, Json::stringValue, -1);
                typename PrimitiveNode<PNK>::iterator
                    found(this->find(key));
                if (found == this->end()) return 0;
                std::string& v(*found->second);
                if (v.empty()) return 0;
                return atol(v.c_str());
            }

            virtual const std::string Name(const std::string& _name) const
                {return _name;}
            virtual ~PrimitiveNode() {
                for (typename ContainerType::iterator it = this->begin();
                it != this->end(); it++)
                    delete it->second;
            }
            virtual void operator=(std::string what) {
                value = what;
            }
            virtual PrimitiveBase& operator()
                (std::string name, std::string value,
                const Json::ValueType& _type = Json::stringValue,
                    int _depth = 1) = 0;

            virtual operator bool() {
                for (typename ContainerType::iterator
                        it = this->begin(); it != this->end(); it++) {
                    PrimitiveBase& n(*it->second);
                    if (!n) return false;
                }
                return true;
            }

            virtual bool propagate(const std::string& txt) {
                for (typename ContainerType::iterator
                        it = this->begin(); it != this->end(); it++) {
                    PrimitiveBase& n(*it->second);
                    if (!n.propagate(txt)) return false;
                }
                return true;
            }

            virtual const std::string GetStringValue() { return value; }

            virtual const std::string GetValue
                (const Json::ValueType& _type) const {
                if (_type == Json::objectValue) return "";
                if (_type == Json::arrayValue) return "";
                if (_type == Json::stringValue) return quoted(value);
                std::stringstream def;
                def << value;
                std::string v(def.str());
                return v;
            }

            virtual const std::string CData(const std::string what) const {
                if (what.empty())
                    return what;
                std::stringstream ss;
                ss << "<![CDATA[" << what << "]]>";
                std::string ret;
                ret = ss.str();
                return ret;
            }

            virtual std::ostream& print(std::ostream& o,
                const Json::ValueType& _type) const {
                if (_type == Json::objectValue)
                    o << std::endl << tabs(depth) << "{";
                if (_type == Json::arrayValue)
                    o << std::endl << tabs(depth) << "[";
                o << GetValue(_type);
                for (typename ContainerType::const_iterator
                        it = this->begin(); it != this->end(); it++) {
                    if (it != this->begin()) o << ",";
                    o << std::endl << tabs(depth+1);
                    const std::string _name(it->first.name());
                    if (_type != Json::arrayValue) o << quoted(Name(_name));
                    if (_type != Json::arrayValue) o << ":";
                    const Json::ValueType type(it->first);
                    (*it->second).print(o, type);
                }

                if (_type == Json::arrayValue)
                    o << std::endl << tabs(depth) << "]";
                if (_type == Json::objectValue)
                    o << std::endl << tabs(depth) << "}";
                return o;
            }

            virtual std::ostream& printxml(std::ostream& o,
                const Json::ValueType&) const {
                o << CData(value);
                for (typename ContainerType::const_iterator
                        it = this->begin(); it != this->end(); it++) {
                    std::string _name(it->first.name());
                    if (!_name.empty())
                        if (isdigit(_name[0]))
                            _name.insert(0, "_");
                    o << "<" << _name << ">";
                    const Json::ValueType type(it->first);
                    (*it->second).printxml(o, type);
                    o << "</" << _name << ">" << std::endl;
                }
                return o;
            }
            virtual operator std::string& () { return value;}
            int Depth(int newdepth=-1) const {
                if (newdepth == -1) return depth;
                depth = newdepth;
                return depth;
            }

            protected:
            std::string value;
            protected:
            virtual const std::string tabs(int _depth) const {
                std::stringstream ss;
                for (int j = 0; j < _depth; j++)
                    ss << " ";
                std::string ret(ss.str());
                return ret;
            }
            mutable int depth;

            protected:
            friend std::ostream& operator<<(std::ostream&,
                const PrimitiveBase&);
            virtual std::ostream& operator<<(std::ostream& o) const
                { return print(o, Json::objectValue); }
        };

    inline std::ostream& operator<<(std::ostream& o, const PrimitiveBase& n)
        {return n.operator<<(o);}

    template <typename KT>
        struct PrimitiveRules : PrimitiveNode<KT> {
            explicit PrimitiveRules(PrimitiveBase* _parent)
                : PrimitiveNode<KT>(_parent) {}
            virtual ~PrimitiveRules() {}
            virtual PrimitiveBase* generate(PrimitiveBase* creator,
                const std::string& _name) = 0;
            virtual bool Describe(PrimitiveBase& descnode) {
                static_cast<void>(descnode);
                return false;
            }
            virtual bool Describe() {
                PrimitiveBase* n(this->Local("Describe"));
                if (!n) return false;
                return Describe(*n);
            }
            virtual PrimitiveBase& operator()(std::string What) {
                const KT key(What, Json::stringValue);
                typename PrimitiveNode<KT>::iterator
                    found(this->find(key));
                if (found == this->end()) {
                    if (this->parent)
                        return (static_cast<PrimitiveNode<KT>&>
                            (*this->parent))(What);
                    else
                        throw What;
                }
                if (!found->second) throw What;
                return *found->second;
            }

            virtual PrimitiveBase& operator()
                (std::string name, std::string value,
                    const Json::ValueType& _type = Json::stringValue,
                    int _depth = 1) {
                const KT key(name, _type);
                PrimitiveRules& me(*this);
                typename PrimitiveNode<KT>::iterator found(this->find(key));
                if (found != this->end())
                    return *found->second;
                PrimitiveBase* G(static_cast<PrimitiveBase*>(
                    generate(this, name)));
                PrimitiveBase& M(*G);
                M = value;
                M.Depth(_depth);
                me[key]=G;
                return M;
            }
        };

    template <typename NODETYPE>
        struct JSonTree : PrimitiveBase {
            virtual NODETYPE& operator()(std::string what) {
                    throw what;
            }
            virtual operator bool() {
                Json::Value jsonroot;
                Json::Reader reader;
                if (!reader.parse(input.c_str(), jsonroot))
                    throw std::runtime_error(
                        reader.getFormattedErrorMessages());
                Root.Create(jsonroot);
                return true;
            }
            virtual bool Load(const std::string& txt) {
                Json::Value jsonroot;
                Json::Reader reader;
                if (!reader.parse(txt.c_str(), jsonroot))
                    throw std::runtime_error(
                        reader.getFormattedErrorMessages());
                Root.Create(jsonroot);
                return true;
            }

            explicit JSonTree(const std::string& _input)
                : PrimitiveBase(NULL), Root(this), input(_input) {}
            virtual void Create(
                Json::Value& json, const int _depth = 0, int order = -1) {
                static_cast<void>(json);
                static_cast<void>(_depth);
                static_cast<void>(order);
                throw "TBD- This could work, but it isn't needed right now";
            }

            operator NODETYPE& () {return Root;}
            virtual std::ostream& printxml(std::ostream& o) const {
                return Root.printxml(o, Json::objectValue);
            }

            protected:
            NODETYPE Root;

            private:
            virtual NODETYPE* generate
                (PrimitiveBase* creator, const std::string&)
                    { return new NODETYPE(creator); }
            const std::string input;
        };
}  //  JCpp
#endif  // INCLUDE_MBCOREUTILS_JCPP_H_
