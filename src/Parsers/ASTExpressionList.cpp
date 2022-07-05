#include <Parsers/ASTExpressionList.h>
#include <IO/Operators.h>
#include <Parsers/ASTIdentifier.h>


namespace DB
{

ASTPtr ASTExpressionList::clone() const
{
    auto clone = std::make_shared<ASTExpressionList>(*this);
    clone->cloneChildren();
    return clone;
}

void ASTExpressionList::formatImpl(const FormatSettings & settings, FormatState & state, FormatStateStacked frame) const
{
    if (frame.expression_list_prepend_whitespace)
        settings.ostr << ' ';

    for (ASTs::const_iterator it = children.begin(); it != children.end(); ++it)
    {
        if (it != children.begin())
        {
            if (separator)
                settings.ostr << separator;
            settings.ostr << ' ';
        }

        if (frame.surround_each_list_element_with_parens)
            settings.ostr << "(";

        FormatStateStacked frame_nested = frame;
        frame_nested.surround_each_list_element_with_parens = false;
        (*it)->formatImpl(settings, state, frame_nested);

        if (frame.surround_each_list_element_with_parens)
            settings.ostr << ")";
    }
}

void ASTExpressionList::formatImplMultiline(const FormatSettings & settings, FormatState & state, FormatStateStacked frame) const
{
    std::string indent_str = "\n" + std::string(4 * (frame.indent + 1), ' ');

    if (frame.expression_list_prepend_whitespace)
    {
        if (!(children.size() > 1 || frame.expression_list_always_start_on_new_line))
            settings.ostr << ' ';
    }

    ++frame.indent;

    for (ASTs::const_iterator it = children.begin(); it != children.end(); ++it)
    {
        if (it != children.begin())
        {
            if (separator)
                settings.ostr << separator;
        }

        if (children.size() > 1 || frame.expression_list_always_start_on_new_line)
            settings.ostr << indent_str;

        FormatStateStacked frame_nested = frame;
        frame_nested.expression_list_always_start_on_new_line = false;
        frame_nested.surround_each_list_element_with_parens = false;

        if (frame.surround_each_list_element_with_parens)
            settings.ostr << "(";

        (*it)->formatImpl(settings, state, frame_nested);

        if (frame.surround_each_list_element_with_parens)
            settings.ostr << ")";
    }
}

void ASTExpressionList::freeSchemaRewrite() 
{
    if (children.size()>=1) {
        ASTs::const_iterator it = children.begin();
        auto isDatatimeOrderBy = false;
        if (((*it)->getID().compare("OrderByElement")==0) && ((*it)->children.front()->getColumnName().compare("datatime")==0)) {
            auto new_time = (*it)->clone() ;
            auto set_new_time = new_time->children.front()->as<ASTIdentifier>();
            set_new_time->setShortName("_sort_time");

            children.insert(children.begin(), new_time);
            isDatatimeOrderBy = true;
        }

        auto i = 0;
        for (ASTs::const_iterator it2 = children.begin(); it2 != children.end(); ++it2)
        {
            if (!isDatatimeOrderBy || (isDatatimeOrderBy && i>=2)) {
                (*it2)->freeSchemaRewrite();
            }

            i++;
        }
    }
}

}
