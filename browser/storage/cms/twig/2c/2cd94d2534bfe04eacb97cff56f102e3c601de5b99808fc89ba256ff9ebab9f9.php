<?php

/* D:\MgcBrowser/themes/magnachain/pages/all-blocks.htm */
class __TwigTemplate_73243a529b40f6e7a584b50f6f9e96ef13d4505f2a4b87535491e097e79b80d8 extends Twig_Template
{
    private $source;

    public function __construct(Twig_Environment $env)
    {
        parent::__construct($env);

        $this->source = $this->getSourceContext();

        $this->parent = false;

        $this->blocks = [
        ];
    }

    protected function doDisplay(array $context, array $blocks = [])
    {
        // line 1
        echo "<style>
   * {margin: 0;padding: 0}
   #calendar {width: 280px;margin: 0px auto; overflow: hidden; border: 1px solid #000; padding: 20px; position: relative}
   #calendar h4 {text-align: center;margin-bottom: 10px}
   #calendar .a1 {position: absolute;top: 20px;left: 20px;}
   #calendar .a2 {position: absolute;top: 20px;right: 20px;}
   #calendar .week {height: 30px;line-height: 20px;border-bottom: 1px solid #000;margin-bottom: 10px}
   #calendar .week li {float: left;width: 30px;height: 30px;text-align: center;list-style: none;}
   #calendar .dateList {overflow: hidden;clear: both}
   #calendar .dateList button {float: left;width: 30px;height: 30px;text-align: center;line-height: 30px;list-style: none;}
   #calendar .dateList .ccc {color: #ccc;}
   #calendar .dateList .red {background: #F90;color: #fff;}
   #calendar .dateList .sun {color: #f00;}
</style>

<div class=\"container content\">

\t<div>

\t\t<p id=\"web_date\" year=\"";
        // line 20
        echo twig_escape_filter($this->env, ($context["year"] ?? null), "html", null, true);
        echo "\" month=\"";
        echo twig_escape_filter($this->env, ($context["month"] ?? null), "html", null, true);
        echo "\" day=\"";
        echo twig_escape_filter($this->env, ($context["day"] ?? null), "html", null, true);
        echo "\" style=\"visibility: hidden;\"></p>

\t\t<div class=\"hidden-xs blockList-cal\" style=\"margin-top: -120px;\">

\t\t\t<ul class=\"ul-date aaa\" style=\"list-style-type: none; text-align: left;\">

\t\t\t\t<span class=\"blockList-cal-unit\" translate style=\"float: left;\">
\t\t\t\t\t<span class=\"ng-scope\">";
        // line 27
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["年"]);
        echo "</span>
\t\t\t\t</span>

\t\t\t\t<li class=\"blockList-cal-row blockList-cal-row-year year\" style=\"margin-bottom: 8px;\"> 
\t\t\t\t\t
\t\t\t\t</li>

\t\t\t\t<span class=\"blockList-cal-unit\" translate style=\"float: left;\">
\t\t\t\t\t<span class=\"ng-scope\">";
        // line 35
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["月"]);
        echo "</span>
\t\t\t\t</span>

\t\t\t\t<li class=\"blockList-cal-row blockList-cal-row-month month\" style=\"margin-bottom: 8px;\">
\t\t\t\t\t
\t\t\t\t</li>

\t\t\t\t<span class=\"blockList-cal-unit\" translate style=\"display: inline;\">
\t\t\t\t\t<span class=\"ng-scope\">";
        // line 43
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["日"]);
        echo "</span>
\t\t\t\t</span>

\t\t\t\t<li class=\"blockList-cal-row blockList-cal-row-date day\" style=\"display: inline;\">
\t\t\t\t\t
\t\t\t\t</li>

\t\t\t</ul>

\t\t</div>

\t</div>
\t
\t<div class=\"block\">
\t\t
\t\t<span class=\"ng-scope date\" style=\"font-size: 24px; float: left;\"></span>

\t\t\t<div class=\"table-responsive\">

\t\t\t\t<table class=\"table table-striped\">

\t\t\t\t\t<thead>
\t\t\t\t\t   <th class=\"\">";
        // line 65
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["高度"]);
        echo "</th>
\t\t\t\t\t   <th class=\"\">";
        // line 66
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["时间"]);
        echo "</th>
\t\t\t\t\t   <th class=\"\">";
        // line 67
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["交易数"]);
        echo "</th>
\t\t\t\t\t   <th class=\"\">";
        // line 68
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["大小"]);
        echo "</th>
\t\t\t\t\t   <th class=\"\">";
        // line 69
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["块哈希"]);
        echo "</th>
\t\t\t\t\t</thead>

\t\t\t\t\t<tbody class=\"tbody-block\">

\t\t\t\t\t\t";
        // line 74
        $context['_parent'] = $context;
        $context['_seq'] = twig_ensure_traversable(($context["infos"] ?? null));
        foreach ($context['_seq'] as $context["_key"] => $context["info"]) {
            // line 75
            echo "
\t\t\t\t\t\t<tr> 
\t\t\t\t\t        <td class=\"\" style=\"line-height: 51px;\">
\t\t\t\t\t        \t<a href=\"/block/";
            // line 78
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "height", []), "html", null, true);
            echo "\">
\t\t\t\t\t\t\t\t\t<span class=\"ellipsis ng-binding\"> ";
            // line 79
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "height", []), "html", null, true);
            echo "</span>
\t\t\t\t\t\t\t\t</a></td>
\t\t\t\t\t        <td class=\"\" style=\"text-align: left; line-height: 51px;\"> ";
            // line 81
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "time", []), "html", null, true);
            echo " </td>
\t\t\t\t\t        <td class=\"\" style=\"text-align: left; line-height: 51px;\"> ";
            // line 82
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "num", []), "html", null, true);
            echo " </td>
\t\t\t\t\t        <td class=\"\" style=\"text-align: left; line-height: 51px;\"> ";
            // line 83
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "blocksize", []), "html", null, true);
            echo " </td>
\t\t\t\t\t        <td class=\"\" style=\"text-align: left; line-height: 51px;\">
\t\t\t\t\t        \t<a href=\"/block/";
            // line 85
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "blockhash", []), "html", null, true);
            echo "\" style=\"float: left;\">
\t\t\t\t\t\t\t\t<span class=\"ellipsis ng-binding\" style=\"white-space:nowrap;\"> ";
            // line 86
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "blockhash", []), "html", null, true);
            echo " </span>
\t\t\t\t\t\t\t\t</a>
\t\t\t\t\t\t\t</td>
\t\t\t\t\t    </tr>

\t\t\t\t\t    ";
        }
        $_parent = $context['_parent'];
        unset($context['_seq'], $context['_iterated'], $context['_key'], $context['info'], $context['_parent'], $context['loop']);
        $context = array_intersect_key($context, $_parent) + $_parent;
        // line 92
        echo "
\t\t\t\t    </tbody>

\t\t\t\t</table>

\t\t\t</div>

\t\t\t";
        // line 99
        if (($context["nothing"] ?? null)) {
            // line 100
            echo "\t\t\t\t\t    
\t\t\t<h1 id=\"no-block\" style=\"margin-top: 100px;\">";
            // line 101
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["没有区块产生~"]);
            echo "</h1>

\t\t\t";
        } else {
            // line 104
            echo "
\t\t\t<h1 id=\"no-block\" style=\"margin-top: 100px; display: none;\">";
            // line 105
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["没有区块产生~"]);
            echo "</h1>
\t\t\t\t
\t\t    ";
        }
        // line 108
        echo "

\t\t\t";
        // line 110
        if (($context["more_block"] ?? null)) {
            // line 111
            echo "
\t\t\t<button class=\"btn btn-default btn-md\" id=\"show-more\" date=\"";
            // line 112
            echo twig_escape_filter($this->env, ($context["year"] ?? null), "html", null, true);
            echo "-";
            echo twig_escape_filter($this->env, ($context["month"] ?? null), "html", null, true);
            echo "-";
            echo twig_escape_filter($this->env, ($context["day"] ?? null), "html", null, true);
            echo "\" style=\"float: left;\">";
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["显示更多"]);
            echo "</button>

\t\t\t";
        }
        // line 115
        echo "
\t</div>

</div>

<script>

\t\$('.input-daterange input').each(function() {
\t    \$(this).datepicker({
\t\t\t\t\t\t\t    format: {
\t\t\t\t\t\t\t        
\t\t\t\t\t\t\t        toDisplay: function (date, format, language) {
\t\t\t\t\t\t\t            var d = new Date(date);
\t\t\t\t\t\t\t            d.setDate(d.getDate());
\t\t\t\t\t\t\t            return d.toISOString();
\t\t\t\t\t\t\t        },

\t\t\t\t\t\t\t        toValue: function (date, format, language) {
\t\t\t\t\t\t\t            var d = new Date(date);
\t\t\t\t\t\t\t            d.setDate(d.getDate());
\t\t\t\t\t\t\t            return new Date(d);
\t\t\t\t\t\t\t        }
\t\t\t\t\t\t\t        
\t\t\t\t\t\t\t    }
\t\t\t\t\t\t\t});

\t    \$(this).on('changeDate', function(){

\t    \tvar timestamp = \$('#datepicker').val();
\t    \tvar date = new Date(timestamp);
\t    \tdate.setHours('00');

\t    \tyear = date.getFullYear().toString();
\t    \tmonth = (date.getMonth()+1).toString();
\t    \tday = date.getDate().toString();

\t    \tvar url = year + '-' + month + '-' + day;

\t    \tif (day.length == 2) {
\t\t\t\thistory.pushState(\"\", \"\", '/all-blocks/' + year + '-' + month + '-' + day);
\t\t\t} else {
\t\t\t\thistory.pushState(\"\", \"\", '/all-blocks/' + year + '-' + month + '-0' + day);
\t\t\t}

\t    \t\$.post(\"/date_block\", {year:year, month:month, day:day}, function (data, status) {

    \t       if (data != 0) {

\t\t\t\t\t\$('.tbody-block').empty();
\t\t\t\t\t\$('#no-block').hide();

\t\t\t\t\tvar data = JSON.parse(data);
\t\t\t\t\t\$('#show-more').remove();

\t\t\t\t\tfor (var i = 0; i < data['block'].length; i++) {

\t\t\t\t\t\t\$('.tbody-block').append(   \"<tr>\" +
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='line-height: 51px;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\"+data['block'][i]['height']+\"'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding'>\"+ data['block'][i]['height']+\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a></td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data['block'][i]['time'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data['block'][i]['num'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data['block'][i]['bits'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\" +data['block'][i]['blockhash']+ \"' style='float: left;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding' style='white-space:nowrap;'>\"+ data['block'][i]['blockhash'] +\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t    \"</tr>\" );
\t\t\t\t\t}

\t\t\t\t\tif (data['more_block'] == 1) {
\t\t\t\t\t\t\$(\".block\").append(\"<button class='btn btn-default btn-md' id='show-more' date='\"+data['date']+\"' style='float: left;'>Show More</button>\")
\t\t\t\t\t}

\t\t\t\t} else {

\t\t\t\t\t\$('.tbody-block').empty();
\t\t\t\t\t\$('#show-more').remove();
\t\t\t\t\t\$('#no-block').show();

\t\t\t\t}

\t\t\t});

\t    });

\t});

\t\$(document).on(\"click\",\"#show-more\",function(){

\t\tvar date = \$('#show-more').attr('date');

\t\t\$.post('/more_block', {date:date}, function(data,status){

\t\t\tif (data != null) {

\t\t\t\tvar data = JSON.parse(data);

\t\t\t\tfor (var i = 0; i < data.length; i++) {

\t\t\t\t\t\$('.tbody-block').append(   \"<tr>\" +
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='line-height: 51px;'>\"+
\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\"+data[i]['height']+\"'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding'>\"+ data[i]['height']+\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a></td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data[i]['time'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data[i]['num'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data[i]['bits'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+
\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\" +data[i]['blockhash']+ \"' style='float: left;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding' style='white-space:nowrap;'>\"+ data[i]['blockhash'] +\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t    \"</tr>\" );
\t\t\t\t}

\t\t\t}

\t\t\t\$('#show-more').remove();

\t\t});

\t});

</script>";
    }

    public function getTemplateName()
    {
        return "D:\\MgcBrowser/themes/magnachain/pages/all-blocks.htm";
    }

    public function isTraitable()
    {
        return false;
    }

    public function getDebugInfo()
    {
        return array (  226 => 115,  214 => 112,  211 => 111,  209 => 110,  205 => 108,  199 => 105,  196 => 104,  190 => 101,  187 => 100,  185 => 99,  176 => 92,  164 => 86,  160 => 85,  155 => 83,  151 => 82,  147 => 81,  142 => 79,  138 => 78,  133 => 75,  129 => 74,  121 => 69,  117 => 68,  113 => 67,  109 => 66,  105 => 65,  80 => 43,  69 => 35,  58 => 27,  44 => 20,  23 => 1,);
    }

    public function getSourceContext()
    {
        return new Twig_Source("<style>
   * {margin: 0;padding: 0}
   #calendar {width: 280px;margin: 0px auto; overflow: hidden; border: 1px solid #000; padding: 20px; position: relative}
   #calendar h4 {text-align: center;margin-bottom: 10px}
   #calendar .a1 {position: absolute;top: 20px;left: 20px;}
   #calendar .a2 {position: absolute;top: 20px;right: 20px;}
   #calendar .week {height: 30px;line-height: 20px;border-bottom: 1px solid #000;margin-bottom: 10px}
   #calendar .week li {float: left;width: 30px;height: 30px;text-align: center;list-style: none;}
   #calendar .dateList {overflow: hidden;clear: both}
   #calendar .dateList button {float: left;width: 30px;height: 30px;text-align: center;line-height: 30px;list-style: none;}
   #calendar .dateList .ccc {color: #ccc;}
   #calendar .dateList .red {background: #F90;color: #fff;}
   #calendar .dateList .sun {color: #f00;}
</style>

<div class=\"container content\">

\t<div>

\t\t<p id=\"web_date\" year=\"{{ year }}\" month=\"{{ month }}\" day=\"{{ day }}\" style=\"visibility: hidden;\"></p>

\t\t<div class=\"hidden-xs blockList-cal\" style=\"margin-top: -120px;\">

\t\t\t<ul class=\"ul-date aaa\" style=\"list-style-type: none; text-align: left;\">

\t\t\t\t<span class=\"blockList-cal-unit\" translate style=\"float: left;\">
\t\t\t\t\t<span class=\"ng-scope\">{{ '年'|_ }}</span>
\t\t\t\t</span>

\t\t\t\t<li class=\"blockList-cal-row blockList-cal-row-year year\" style=\"margin-bottom: 8px;\"> 
\t\t\t\t\t
\t\t\t\t</li>

\t\t\t\t<span class=\"blockList-cal-unit\" translate style=\"float: left;\">
\t\t\t\t\t<span class=\"ng-scope\">{{ '月'|_ }}</span>
\t\t\t\t</span>

\t\t\t\t<li class=\"blockList-cal-row blockList-cal-row-month month\" style=\"margin-bottom: 8px;\">
\t\t\t\t\t
\t\t\t\t</li>

\t\t\t\t<span class=\"blockList-cal-unit\" translate style=\"display: inline;\">
\t\t\t\t\t<span class=\"ng-scope\">{{ '日'|_ }}</span>
\t\t\t\t</span>

\t\t\t\t<li class=\"blockList-cal-row blockList-cal-row-date day\" style=\"display: inline;\">
\t\t\t\t\t
\t\t\t\t</li>

\t\t\t</ul>

\t\t</div>

\t</div>
\t
\t<div class=\"block\">
\t\t
\t\t<span class=\"ng-scope date\" style=\"font-size: 24px; float: left;\"></span>

\t\t\t<div class=\"table-responsive\">

\t\t\t\t<table class=\"table table-striped\">

\t\t\t\t\t<thead>
\t\t\t\t\t   <th class=\"\">{{ '高度'|_ }}</th>
\t\t\t\t\t   <th class=\"\">{{ '时间'|_ }}</th>
\t\t\t\t\t   <th class=\"\">{{ '交易数'|_ }}</th>
\t\t\t\t\t   <th class=\"\">{{ '大小'|_ }}</th>
\t\t\t\t\t   <th class=\"\">{{ '块哈希'|_ }}</th>
\t\t\t\t\t</thead>

\t\t\t\t\t<tbody class=\"tbody-block\">

\t\t\t\t\t\t{% for info in infos %}

\t\t\t\t\t\t<tr> 
\t\t\t\t\t        <td class=\"\" style=\"line-height: 51px;\">
\t\t\t\t\t        \t<a href=\"/block/{{ info.height }}\">
\t\t\t\t\t\t\t\t\t<span class=\"ellipsis ng-binding\"> {{ info.height }}</span>
\t\t\t\t\t\t\t\t</a></td>
\t\t\t\t\t        <td class=\"\" style=\"text-align: left; line-height: 51px;\"> {{ info.time }} </td>
\t\t\t\t\t        <td class=\"\" style=\"text-align: left; line-height: 51px;\"> {{ info.num }} </td>
\t\t\t\t\t        <td class=\"\" style=\"text-align: left; line-height: 51px;\"> {{ info.blocksize }} </td>
\t\t\t\t\t        <td class=\"\" style=\"text-align: left; line-height: 51px;\">
\t\t\t\t\t        \t<a href=\"/block/{{ info.blockhash }}\" style=\"float: left;\">
\t\t\t\t\t\t\t\t<span class=\"ellipsis ng-binding\" style=\"white-space:nowrap;\"> {{ info.blockhash }} </span>
\t\t\t\t\t\t\t\t</a>
\t\t\t\t\t\t\t</td>
\t\t\t\t\t    </tr>

\t\t\t\t\t    {% endfor %}

\t\t\t\t    </tbody>

\t\t\t\t</table>

\t\t\t</div>

\t\t\t{% if nothing %}
\t\t\t\t\t    
\t\t\t<h1 id=\"no-block\" style=\"margin-top: 100px;\">{{ '没有区块产生~'|_ }}</h1>

\t\t\t{% else %}

\t\t\t<h1 id=\"no-block\" style=\"margin-top: 100px; display: none;\">{{ '没有区块产生~'|_ }}</h1>
\t\t\t\t
\t\t    {% endif %}


\t\t\t{% if more_block %}

\t\t\t<button class=\"btn btn-default btn-md\" id=\"show-more\" date=\"{{ year }}-{{ month }}-{{ day }}\" style=\"float: left;\">{{ '显示更多'|_ }}</button>

\t\t\t{% endif %}

\t</div>

</div>

<script>

\t\$('.input-daterange input').each(function() {
\t    \$(this).datepicker({
\t\t\t\t\t\t\t    format: {
\t\t\t\t\t\t\t        
\t\t\t\t\t\t\t        toDisplay: function (date, format, language) {
\t\t\t\t\t\t\t            var d = new Date(date);
\t\t\t\t\t\t\t            d.setDate(d.getDate());
\t\t\t\t\t\t\t            return d.toISOString();
\t\t\t\t\t\t\t        },

\t\t\t\t\t\t\t        toValue: function (date, format, language) {
\t\t\t\t\t\t\t            var d = new Date(date);
\t\t\t\t\t\t\t            d.setDate(d.getDate());
\t\t\t\t\t\t\t            return new Date(d);
\t\t\t\t\t\t\t        }
\t\t\t\t\t\t\t        
\t\t\t\t\t\t\t    }
\t\t\t\t\t\t\t});

\t    \$(this).on('changeDate', function(){

\t    \tvar timestamp = \$('#datepicker').val();
\t    \tvar date = new Date(timestamp);
\t    \tdate.setHours('00');

\t    \tyear = date.getFullYear().toString();
\t    \tmonth = (date.getMonth()+1).toString();
\t    \tday = date.getDate().toString();

\t    \tvar url = year + '-' + month + '-' + day;

\t    \tif (day.length == 2) {
\t\t\t\thistory.pushState(\"\", \"\", '/all-blocks/' + year + '-' + month + '-' + day);
\t\t\t} else {
\t\t\t\thistory.pushState(\"\", \"\", '/all-blocks/' + year + '-' + month + '-0' + day);
\t\t\t}

\t    \t\$.post(\"/date_block\", {year:year, month:month, day:day}, function (data, status) {

    \t       if (data != 0) {

\t\t\t\t\t\$('.tbody-block').empty();
\t\t\t\t\t\$('#no-block').hide();

\t\t\t\t\tvar data = JSON.parse(data);
\t\t\t\t\t\$('#show-more').remove();

\t\t\t\t\tfor (var i = 0; i < data['block'].length; i++) {

\t\t\t\t\t\t\$('.tbody-block').append(   \"<tr>\" +
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='line-height: 51px;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\"+data['block'][i]['height']+\"'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding'>\"+ data['block'][i]['height']+\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a></td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data['block'][i]['time'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data['block'][i]['num'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data['block'][i]['bits'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\" +data['block'][i]['blockhash']+ \"' style='float: left;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding' style='white-space:nowrap;'>\"+ data['block'][i]['blockhash'] +\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t    \"</tr>\" );
\t\t\t\t\t}

\t\t\t\t\tif (data['more_block'] == 1) {
\t\t\t\t\t\t\$(\".block\").append(\"<button class='btn btn-default btn-md' id='show-more' date='\"+data['date']+\"' style='float: left;'>Show More</button>\")
\t\t\t\t\t}

\t\t\t\t} else {

\t\t\t\t\t\$('.tbody-block').empty();
\t\t\t\t\t\$('#show-more').remove();
\t\t\t\t\t\$('#no-block').show();

\t\t\t\t}

\t\t\t});

\t    });

\t});

\t\$(document).on(\"click\",\"#show-more\",function(){

\t\tvar date = \$('#show-more').attr('date');

\t\t\$.post('/more_block', {date:date}, function(data,status){

\t\t\tif (data != null) {

\t\t\t\tvar data = JSON.parse(data);

\t\t\t\tfor (var i = 0; i < data.length; i++) {

\t\t\t\t\t\$('.tbody-block').append(   \"<tr>\" +
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='line-height: 51px;'>\"+
\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\"+data[i]['height']+\"'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding'>\"+ data[i]['height']+\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a></td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data[i]['time'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data[i]['num'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data[i]['bits'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+
\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\" +data[i]['blockhash']+ \"' style='float: left;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding' style='white-space:nowrap;'>\"+ data[i]['blockhash'] +\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t    \"</tr>\" );
\t\t\t\t}

\t\t\t}

\t\t\t\$('#show-more').remove();

\t\t});

\t});

</script>", "D:\\MgcBrowser/themes/magnachain/pages/all-blocks.htm", "");
    }
}
