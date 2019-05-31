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
        echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"/themes/magnachain/assets/css/all-blocks-loading.css\">
<style>
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
        // line 21
        echo twig_escape_filter($this->env, ($context["year"] ?? null), "html", null, true);
        echo "\" month=\"";
        echo twig_escape_filter($this->env, ($context["month"] ?? null), "html", null, true);
        echo "\" day=\"";
        echo twig_escape_filter($this->env, ($context["day"] ?? null), "html", null, true);
        echo "\" date=\"";
        echo twig_escape_filter($this->env, ($context["year"] ?? null), "html", null, true);
        echo "-";
        echo twig_escape_filter($this->env, ($context["month"] ?? null), "html", null, true);
        echo "-";
        echo twig_escape_filter($this->env, ($context["day"] ?? null), "html", null, true);
        echo "\" style=\"visibility: hidden;\"></p>

\t\t<div class=\"hidden-xs blockList-cal\" style=\"margin-top: -120px;\">

\t\t\t<ul class=\"ul-date aaa\" style=\"list-style-type: none; text-align: left;\">

\t\t\t\t<span class=\"blockList-cal-unit\" translate style=\"float: left;\">
\t\t\t\t\t<span class=\"ng-scope\">";
        // line 28
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["年"]);
        echo "</span>
\t\t\t\t</span>

\t\t\t\t<li class=\"blockList-cal-row blockList-cal-row-year year\" style=\"margin-bottom: 8px;\"> 
\t\t\t\t\t
\t\t\t\t</li>

\t\t\t\t<span class=\"blockList-cal-unit\" translate style=\"float: left;\">
\t\t\t\t\t<span class=\"ng-scope\">";
        // line 36
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["月"]);
        echo "</span>
\t\t\t\t</span>

\t\t\t\t<li class=\"blockList-cal-row blockList-cal-row-month month\" style=\"margin-bottom: 8px;\">
\t\t\t\t\t
\t\t\t\t</li>

\t\t\t\t<span class=\"blockList-cal-unit\" translate style=\"display: inline;\">
\t\t\t\t\t<span class=\"ng-scope\">";
        // line 44
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["日"]);
        echo "</span>
\t\t\t\t</span>

\t\t\t\t<li class=\"blockList-cal-row blockList-cal-row-date day\" style=\"display: inline;\">
\t\t\t\t\t
\t\t\t\t</li>

\t\t\t</ul>

\t\t</div>

\t</div>

\t<div class=\"block\">
\t\t
\t\t<span class=\"ng-scope date\" style=\"font-size: 24px; float: left;\"></span>

\t\t\t<div class=\"table-responsive\">

\t\t\t\t<table class=\"table table-striped\">

\t\t\t\t\t<thead>
\t\t\t\t\t   <th class=\"\">";
        // line 66
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["高度"]);
        echo "</th>
\t\t\t\t\t   <th class=\"\">";
        // line 67
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["时间"]);
        echo "</th>
\t\t\t\t\t   <th class=\"\">";
        // line 68
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["交易数"]);
        echo "</th>
\t\t\t\t\t   <th class=\"\">";
        // line 69
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["大小"]);
        echo "</th>
\t\t\t\t\t   <th class=\"\">";
        // line 70
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["块哈希"]);
        echo "</th>
\t\t\t\t\t</thead>
\t\t\t\t\t
\t\t\t\t\t<tbody class=\"tbody-block\">
\t\t\t\t\t\t
\t\t\t\t\t\t";
        // line 75
        $context['_parent'] = $context;
        $context['_seq'] = twig_ensure_traversable(($context["infos"] ?? null));
        foreach ($context['_seq'] as $context["_key"] => $context["info"]) {
            // line 76
            echo "
\t\t\t\t\t\t<tr> 
\t\t\t\t\t        <td class=\"\" style=\"line-height: 51px;\">
\t\t\t\t\t        \t<a href=\"/block/";
            // line 79
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "height", []), "html", null, true);
            echo "\">
\t\t\t\t\t\t\t\t\t<span class=\"ellipsis ng-binding\"> ";
            // line 80
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "height", []), "html", null, true);
            echo "</span>
\t\t\t\t\t\t\t\t</a></td>
\t\t\t\t\t        <td class=\"\" style=\"text-align: left; line-height: 51px;\"> ";
            // line 82
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "time", []), "html", null, true);
            echo " </td>
\t\t\t\t\t        <td class=\"\" style=\"text-align: left; line-height: 51px;\"> ";
            // line 83
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "num", []), "html", null, true);
            echo " </td>
\t\t\t\t\t        <td class=\"\" style=\"text-align: left; line-height: 51px;\"> ";
            // line 84
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "blocksize", []), "html", null, true);
            echo " </td>
\t\t\t\t\t        <td class=\"\" style=\"text-align: left; line-height: 51px;\">
\t\t\t\t\t        \t<a href=\"/block/";
            // line 86
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "blockhash", []), "html", null, true);
            echo "\" style=\"float: left;\">
\t\t\t\t\t\t\t\t<span class=\"ellipsis ng-binding\" style=\"white-space:nowrap;\"> ";
            // line 87
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
        // line 93
        echo "
\t\t\t\t    </tbody>

\t\t\t\t</table>

\t\t\t</div>

\t\t\t<!-- pagination -->

\t\t\t<p id=\"web_page\" page=\"";
        // line 102
        echo twig_escape_filter($this->env, ($context["pagination"] ?? null), "html", null, true);
        echo "\" style=\"visibility: hidden;\"></p>

\t\t\t";
        // line 104
        if ((($context["pagination"] ?? null) > 4)) {
            // line 105
            echo "
\t\t\t<ul class=\"pagination\" id=\"pagination\">
\t\t\t\t<!-- <li><a href=\"javascript:void(0);\" id=\"pagination_previous\" style=\"color: #777777\">&laquo;</a></li> -->
\t\t\t\t";
            // line 108
            $context['_parent'] = $context;
            $context['_seq'] = twig_ensure_traversable(range(1, 5));
            foreach ($context['_seq'] as $context["_key"] => $context["i"]) {
                // line 109
                echo "\t\t\t\t\t";
                if (($context["i"] == 1)) {
                    // line 110
                    echo "\t\t\t\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_1\" page=\"1\" style=\"background-color: #ebebeb; color: #777777\">";
                    echo twig_escape_filter($this->env, $context["i"], "html", null, true);
                    echo "</a></li>
\t\t\t\t\t";
                } else {
                    // line 112
                    echo "\t\t\t\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_";
                    echo twig_escape_filter($this->env, $context["i"], "html", null, true);
                    echo "\" page=\"";
                    echo twig_escape_filter($this->env, $context["i"], "html", null, true);
                    echo "\" style=\"color: #777777\">";
                    echo twig_escape_filter($this->env, $context["i"], "html", null, true);
                    echo "</a></li>
\t\t\t\t\t";
                }
                // line 114
                echo "\t\t\t\t";
            }
            $_parent = $context['_parent'];
            unset($context['_seq'], $context['_iterated'], $context['_key'], $context['i'], $context['_parent'], $context['loop']);
            $context = array_intersect_key($context, $_parent) + $_parent;
            // line 115
            echo "\t\t\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_next\" page=\"2\" style=\"color: #777777\">&raquo;</a></li>
\t\t\t\t<p style=\"display: inline-block;\">&nbsp 共";
            // line 116
            echo twig_escape_filter($this->env, ($context["pagination"] ?? null), "html", null, true);
            echo "页  跳到</p>
\t\t\t\t<input type=\"text\" id=\"page-input\" name=\"page-input\" style=\"width: 54px;\">
\t\t\t\t<p style=\"display: inline-block;\">页</p>
\t\t\t\t<button class=\"btn btn-default\" id=\"confirm-page\" onclick=\"pageInfo();\" style=\"display: inline-block;\">确认</button>
\t\t\t</ul>

\t\t\t";
        } else {
            // line 123
            echo "
\t\t\t<ul class=\"pagination\" id=\"pagination\" style=\"display: none;\">
\t\t\t\t<!-- <li><a href=\"javascript:void(0);\" id=\"pagination_previous\" style=\"color: #777777\">&laquo;</a></li> -->
\t\t\t\t";
            // line 126
            $context['_parent'] = $context;
            $context['_seq'] = twig_ensure_traversable(range(1, ($context["pagination"] ?? null)));
            foreach ($context['_seq'] as $context["_key"] => $context["i"]) {
                // line 127
                echo "\t\t\t\t\t";
                if (($context["i"] == 1)) {
                    // line 128
                    echo "\t\t\t\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_1\" page=\"1\" class=\"avtive\" style=\"background-color: #ebebeb; color: #777777\">";
                    echo twig_escape_filter($this->env, $context["i"], "html", null, true);
                    echo "</a></li>
\t\t\t\t\t";
                } else {
                    // line 130
                    echo "\t\t\t\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_";
                    echo twig_escape_filter($this->env, $context["i"], "html", null, true);
                    echo "\" page=";
                    echo twig_escape_filter($this->env, $context["i"], "html", null, true);
                    echo " style=\"color: #777777\">";
                    echo twig_escape_filter($this->env, $context["i"], "html", null, true);
                    echo "</a></li>
\t\t\t\t\t";
                }
                // line 132
                echo "\t\t\t\t";
            }
            $_parent = $context['_parent'];
            unset($context['_seq'], $context['_iterated'], $context['_key'], $context['i'], $context['_parent'], $context['loop']);
            $context = array_intersect_key($context, $_parent) + $_parent;
            // line 133
            echo "\t\t\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_next\" page=\"2\" style=\"color: #777777\">&raquo;</a></li>
\t\t\t\t<p style=\"display: inline-block;\">&nbsp 共";
            // line 134
            echo twig_escape_filter($this->env, ($context["pagination"] ?? null), "html", null, true);
            echo "页  跳到</p>
\t\t\t\t<input type=\"text\" id=\"page-input\" name=\"page-input\" style=\"width: 54px;\">
\t\t\t\t<p style=\"display: inline-block;\">页</p>
\t\t\t\t<button class=\"btn btn-default\" id=\"confirm-page\" onclick=\"pageInfo();\" style=\"display: inline-block;\">确认</button>
\t\t\t</ul>

\t\t\t";
        }
        // line 141
        echo "
\t\t\t<!-- pagination end --> 
\t\t\t<div id=\"div-loader\" style=\"height:37px; margin-top: 100px; display: none;\">
\t\t\t\t<div class=\"loader\" >Loading...</div>
\t\t\t</div>

\t\t\t";
        // line 147
        if (($context["nothing"] ?? null)) {
            // line 148
            echo "\t\t\t\t\t    
\t\t\t<h1 id=\"no-block\" style=\"margin-top: 100px;\">";
            // line 149
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["没有区块产生~"]);
            echo "</h1>

\t\t\t";
        } else {
            // line 152
            echo "
\t\t\t<h1 id=\"no-block\" style=\"margin-top: 100px; display: none;\">";
            // line 153
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["没有区块产生~"]);
            echo "</h1>
\t\t\t\t
\t\t    ";
        }
        // line 156
        echo "
\t\t\t<!-- ";
        // line 157
        if (($context["more_block"] ?? null)) {
            // line 158
            echo "\t\t\t
\t\t\t<button class=\"btn btn-default btn-md\" id=\"show-more\" date=\"";
            // line 159
            echo twig_escape_filter($this->env, ($context["year"] ?? null), "html", null, true);
            echo "-";
            echo twig_escape_filter($this->env, ($context["month"] ?? null), "html", null, true);
            echo "-";
            echo twig_escape_filter($this->env, ($context["day"] ?? null), "html", null, true);
            echo "\" style=\"float: left;\">";
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["显示更多"]);
            echo "</button>
\t\t\t
\t\t\t";
        }
        // line 161
        echo " -->

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

\t\t\t\t\tfor (var i = 0; i < data['block'].length; i++) {

\t\t\t\t\t\t\$('.tbody-block').append(   \"<tr>\" +
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='line-height: 51px;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\"+data['block'][i]['height']+\"'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding'>\"+ data['block'][i]['height']+\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a></td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data['block'][i]['time'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data['block'][i]['num'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data['block'][i]['blocksize'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\" +data['block'][i]['blockhash']+ \"' style='float: left;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding' style='white-space:nowrap;'>\"+ data['block'][i]['blockhash'] +\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t    \"</tr>\" );
\t\t\t\t\t}

\t\t\t\t\tif (data['pagination']!=null) {console.log(data['pagination']);data['pagination'] = parseInt(data['pagination']);console.log(data['pagination']);}

\t\t\t\t\tif (data['pagination']>1) {

\t\t\t\t\t\t\$(\"#web_page\").attr('page', '');
\t\t\t\t\t\t\$(\"#web_page\").attr('page', data['pagination']);

\t\t\t\t\t\t\$('#pagination').find('li').empty();
\t\t\t\t\t\t\$(\"#pagination p:first\").text(\"\");
\t\t\t\t\t\t\$(\"#pagination p:first\").text('共'+data['pagination']+\"页 跳到\")

\t\t\t\t\t\tif (data['pagination']>4) {
\t\t\t\t\t\t\t
\t\t\t\t\t\t\tfor(var i = 1; i <= 5; i++){

\t\t\t\t\t\t\t\tif (i==1) {
\t\t\t\t\t\t\t\t\t\$('#pagination').append(\"<li><a href='javascript:void(0);' id='pagination_1' page='1' style='background-color: #ebebeb; color: #777777'>1</a></li>\"); 
\t\t\t\t\t\t\t\t}else{
\t\t\t\t\t\t\t\t\t\$('#pagination').append(\"<li><a href='javascript:void(0);' id='pagination_\"+i+\"' page='\"+i+\"' style='color: #777777'>\"+i+\"</a></li>\"); 
\t\t\t\t\t\t\t\t}

\t\t\t\t\t\t\t}

\t\t\t\t\t\t\t\$('#pagination').append(\"<li><a href='javascript:void(0);' id='pagination_next' page='2' style='color: #777777'>&raquo;</a></li>\"); 
\t\t\t\t\t\t\t
\t\t\t\t\t\t} else {

\t\t\t\t\t\t\tfor(var i = 1; i <= data['pagination']; i++){
\t\t\t\t\t\t\t\tif (i==1) {
\t\t\t\t\t\t\t\t\t\$('#pagination').append(\"<li><a href='javascript:void(0);' id='pagination_1' page='1' class='avtive' style='background-color: #ebebeb; color: #777777'>\"+i+\"</a></li>\"); 
\t\t\t\t\t\t\t\t} else {
\t\t\t\t\t\t\t\t\t\$('#pagination').append(\"<li><a href='javascript:void(0);' id='pagination_\"+i+\"' page=\"+i+\" style='color: #777777'>\"+i+\"</a></li>\"); 
\t\t\t\t\t\t\t\t}
\t\t\t\t\t\t\t}

\t\t\t\t\t\t}

\t\t\t\t\t\t\$('#pagination').show();

\t\t\t\t\t} 

\t\t\t\t} else {

\t\t\t\t\t\$(\"#pagination\").hide();
\t\t\t\t\t\$('.tbody-block').empty();
\t\t\t\t\t\$('#show-more').remove();
\t\t\t\t\t\$('#no-block').show();

\t\t\t\t}

\t\t\t});

\t    });

\t});

\t\$(document).on(\"click\",\"#show-more\",function(){

\t\t\$('#show-more').hide();
\t\t\$('#div-loader').show();

\t\tvar date = \$('#show-more').attr('date');

\t\t\$.post('/more_block', {date:date}, function(data,status){

\t\t\tif (data != null) {

\t\t\t\t\$('#div-loader').hide();

\t\t\t\tvar data = JSON.parse(data);

\t\t\t\tfor (var i = 0; i < data.length; i++) {

\t\t\t\t\t\$('.tbody-block').append(   \"<tr>\" +
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='line-height: 51px;'>\"+
\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\"+data[i]['height']+\"'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding'>\"+ data[i]['height']+\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a></td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data[i]['time'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data[i]['num'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data[i]['blocksize'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+
\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\" +data[i]['blockhash']+ \"' style='float: left;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding' style='white-space:nowrap;'>\"+ data[i]['blockhash'] +\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t    \"</tr>\" );

\t\t\t\t}

\t\t\t}

\t\t});

\t});


\t\$(\"#pagination a\").bind(\"click\", function(event){
\t
\t\tevent.stopImmediatePropagation();

\t\tvar page = \$(this).attr('page');
\t\tvar total_page = \$(\"#web_page\").attr('page');
\t\tvar date = \$('#web_date').attr('date');
\t\tconsole.log(page+total_page+date);
\t\tgetPageInfo(page, total_page, date);
\t\t
\t});


\t\$('#page-input').bind('keydown', function(event){

        if(event.keyCode == \"13\")    
        {

        \tvar page = \$(this).val();
        \tvar total_page = \$(\"#web_page\").attr('page');
        \tvar date = \$('#web_date').attr('date');
         \t
         \tif (page != null) {

         \t\tgetPageInfo(page, total_page, date);

         \t}

        }

    });

    function pageInfo(){

    \tvar page = \$('#page-input').val();
    \tvar total_page = \$(\"#web_page\").attr('page');
    \tvar date = \$('#web_date').attr('date');
     \t
     \tif (page != null) {

     \t\tgetPageInfo(page, total_page, date);

     \t}

    }

    function getPageInfo(page, total_page, date){
    \t
    \t\$.get(\"/all_blocks_pagination\", {page:page, total_page:total_page, date:date}, function (data, status) {
   \t\t\t
       \t\tif (data != null) {

       \t\t\t\$('.tbody-block').empty();

       \t\t\tvar data = JSON.parse(data);

       \t\t\tif (total_page <= 5) {

       \t\t\t\t\$(\"#pagination\").find('li').remove();

       \t\t\t\tif (data['page'] > 1) {
       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_previous' page='\"+  (data['page']-1) +\"' style='color: #777777'>&laquo;</a></li>\");
       \t\t\t\t\tregisterClick(\"#pagination_previous\");
       \t\t\t\t}
\t\t\t\t
       \t\t\t\tfor(var i = 0; i < total_page; i++){

       \t\t\t\t\tif (data['page'] == i) {
       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='background-color: #ebebeb; color: #777777'>\"+ i +\"</a></li>\");
       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
       \t\t\t\t\t} else {
       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='color: #777777'>\"+ i +\"</a></li>\");
       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
       \t\t\t\t\t}  

       \t\t\t\t}
\t\t\t\t\t
\t\t\t\t\tif (data['page'] < total_page) {
       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_next' page='\"+ (data['page']+1) +\"' style='color: #777777'>&raquo;</a></li>\");
       \t\t\t\t\tregisterClick(\"#pagination_next\");
       \t\t\t\t}

       \t\t\t} else if (total_page > 5) {

       \t\t\t\tif (total_page-data['page']>=2 && data['page']>2) {  //中间情况

       \t\t\t\t\t\$(\"#pagination\").find('li').remove();

\t       \t\t\t\tif (data['page'] > 1) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_previous' page='\"+ (data['page']-1) +\"' style='color: #777777'>&laquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_previous\");
\t       \t\t\t\t}
\t\t\t\t\t\t
\t       \t\t\t\tfor(var i = data['page']-2; i <= data['page']+2; i++){

\t       \t\t\t\t\tif (data['page'] == i) {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='background-color: #ebebeb; color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t} else {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t}  

\t       \t\t\t\t}
\t\t\t\t\t\t
\t\t\t\t\t\tif (data['page'] < total_page) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_next' page='\"+ (data['page']+1) +\"' style='color: #777777'>&raquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_next\");
\t       \t\t\t\t}

       \t\t\t\t} else if (data['page']<=2){ //最左情况

       \t\t\t\t\t\$(\"#pagination\").find('li').remove();

\t       \t\t\t\tif (data['page'] > 1) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_previous' page='\"+ (data['page']-1) +\"' style='color: #777777'>&laquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_previous\");
\t       \t\t\t\t}
\t\t\t\t\t
\t       \t\t\t\tfor(var i = 1; i <= 5; i++){

\t       \t\t\t\t\tif (data['page'] == i) {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='background-color: #ebebeb; color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t} else {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t}  

\t       \t\t\t\t}
\t\t\t\t\t\t
\t\t\t\t\t\tif (data['page'] < total_page) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_next' page='\"+ (data['page']+1) +\"' style='color: #777777'>&raquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_next\");
\t       \t\t\t\t}


       \t\t\t\t} else if (total_page-data['page']<2){ //最右情况

       \t\t\t\t\t\$(\"#pagination\").find('li').remove();

\t       \t\t\t\tif (data['page'] > 1) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_previous' page='\"+ (data['page']-1) +\"' style='color: #777777'>&laquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_previous\");
\t       \t\t\t\t}
\t\t\t\t\t
\t       \t\t\t\tfor(var i = total_page-4; i <= total_page; i++){

\t       \t\t\t\t\tif (data['page'] == i) {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='background-color: #ebebeb; color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t} else {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t}  

\t       \t\t\t\t}
\t\t\t\t\t\t
\t\t\t\t\t\tif (data['page'] < total_page) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_next' page='\"+ (data['page']+1) +\"' style='color: #777777'>&raquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_next\");
\t       \t\t\t\t}

       \t\t\t\t}

       \t\t\t}

\t\t\t\tfor (var i = 0; i < data['block'].length; i++) {

\t\t\t\t\t\$('.tbody-block').append(   \"<tr>\" +
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='line-height: 51px;'>\"+
\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\"+data['block'][i]['height']+\"'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding'>\"+ data['block'][i]['height']+\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a></td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data['block'][i]['time'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data['block'][i]['num'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data['block'][i]['blocksize'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+
\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\" +data['block'][i]['blockhash']+ \"' style='float: left;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding' style='white-space:nowrap;'>\"+ data['block'][i]['blockhash'] +\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t    \"</tr>\" );

\t\t\t\t}

       \t\t}

\t\t});

    }

    function registerClick(id) {

    \t\$(id).unbind('click');

    \t\$(id).bind(\"click\", function(event){
\t\t
\t\t\tevent.stopImmediatePropagation();

\t\t\tvar page = \$(this).attr('page');
\t\t\tvar total_page = \$(\"#web_page\").attr('page');
\t\t\tvar date = \$('#web_date').attr('date');
\t\t\tgetPageInfo(page, total_page, date);
\t\t\t
\t\t});

    }

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
        return array (  343 => 161,  331 => 159,  328 => 158,  326 => 157,  323 => 156,  317 => 153,  314 => 152,  308 => 149,  305 => 148,  303 => 147,  295 => 141,  285 => 134,  282 => 133,  276 => 132,  266 => 130,  260 => 128,  257 => 127,  253 => 126,  248 => 123,  238 => 116,  235 => 115,  229 => 114,  219 => 112,  213 => 110,  210 => 109,  206 => 108,  201 => 105,  199 => 104,  194 => 102,  183 => 93,  171 => 87,  167 => 86,  162 => 84,  158 => 83,  154 => 82,  149 => 80,  145 => 79,  140 => 76,  136 => 75,  128 => 70,  124 => 69,  120 => 68,  116 => 67,  112 => 66,  87 => 44,  76 => 36,  65 => 28,  45 => 21,  23 => 1,);
    }

    public function getSourceContext()
    {
        return new Twig_Source("<link rel=\"stylesheet\" type=\"text/css\" href=\"/themes/magnachain/assets/css/all-blocks-loading.css\">
<style>
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

\t\t<p id=\"web_date\" year=\"{{ year }}\" month=\"{{ month }}\" day=\"{{ day }}\" date=\"{{ year }}-{{ month }}-{{ day }}\" style=\"visibility: hidden;\"></p>

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
\t\t\t\t\t
\t\t\t\t\t<tbody class=\"tbody-block\">
\t\t\t\t\t\t
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

\t\t\t<!-- pagination -->

\t\t\t<p id=\"web_page\" page=\"{{ pagination }}\" style=\"visibility: hidden;\"></p>

\t\t\t{% if pagination > 4 %}

\t\t\t<ul class=\"pagination\" id=\"pagination\">
\t\t\t\t<!-- <li><a href=\"javascript:void(0);\" id=\"pagination_previous\" style=\"color: #777777\">&laquo;</a></li> -->
\t\t\t\t{% for i in 1..5 %}
\t\t\t\t\t{% if i == 1 %}
\t\t\t\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_1\" page=\"1\" style=\"background-color: #ebebeb; color: #777777\">{{ i }}</a></li>
\t\t\t\t\t{% else %}
\t\t\t\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_{{ i }}\" page=\"{{ i }}\" style=\"color: #777777\">{{ i }}</a></li>
\t\t\t\t\t{% endif %}
\t\t\t\t{% endfor %}
\t\t\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_next\" page=\"2\" style=\"color: #777777\">&raquo;</a></li>
\t\t\t\t<p style=\"display: inline-block;\">&nbsp 共{{ pagination }}页  跳到</p>
\t\t\t\t<input type=\"text\" id=\"page-input\" name=\"page-input\" style=\"width: 54px;\">
\t\t\t\t<p style=\"display: inline-block;\">页</p>
\t\t\t\t<button class=\"btn btn-default\" id=\"confirm-page\" onclick=\"pageInfo();\" style=\"display: inline-block;\">确认</button>
\t\t\t</ul>

\t\t\t{% else %}

\t\t\t<ul class=\"pagination\" id=\"pagination\" style=\"display: none;\">
\t\t\t\t<!-- <li><a href=\"javascript:void(0);\" id=\"pagination_previous\" style=\"color: #777777\">&laquo;</a></li> -->
\t\t\t\t{% for i in 1..pagination %}
\t\t\t\t\t{% if i == 1 %}
\t\t\t\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_1\" page=\"1\" class=\"avtive\" style=\"background-color: #ebebeb; color: #777777\">{{ i }}</a></li>
\t\t\t\t\t{% else %}
\t\t\t\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_{{ i }}\" page={{ i }} style=\"color: #777777\">{{ i }}</a></li>
\t\t\t\t\t{% endif %}
\t\t\t\t{% endfor %}
\t\t\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_next\" page=\"2\" style=\"color: #777777\">&raquo;</a></li>
\t\t\t\t<p style=\"display: inline-block;\">&nbsp 共{{ pagination }}页  跳到</p>
\t\t\t\t<input type=\"text\" id=\"page-input\" name=\"page-input\" style=\"width: 54px;\">
\t\t\t\t<p style=\"display: inline-block;\">页</p>
\t\t\t\t<button class=\"btn btn-default\" id=\"confirm-page\" onclick=\"pageInfo();\" style=\"display: inline-block;\">确认</button>
\t\t\t</ul>

\t\t\t{% endif %}

\t\t\t<!-- pagination end --> 
\t\t\t<div id=\"div-loader\" style=\"height:37px; margin-top: 100px; display: none;\">
\t\t\t\t<div class=\"loader\" >Loading...</div>
\t\t\t</div>

\t\t\t{% if nothing %}
\t\t\t\t\t    
\t\t\t<h1 id=\"no-block\" style=\"margin-top: 100px;\">{{ '没有区块产生~'|_ }}</h1>

\t\t\t{% else %}

\t\t\t<h1 id=\"no-block\" style=\"margin-top: 100px; display: none;\">{{ '没有区块产生~'|_ }}</h1>
\t\t\t\t
\t\t    {% endif %}

\t\t\t<!-- {% if more_block %}
\t\t\t
\t\t\t<button class=\"btn btn-default btn-md\" id=\"show-more\" date=\"{{ year }}-{{ month }}-{{ day }}\" style=\"float: left;\">{{ '显示更多'|_ }}</button>
\t\t\t
\t\t\t{% endif %} -->

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

\t\t\t\t\tfor (var i = 0; i < data['block'].length; i++) {

\t\t\t\t\t\t\$('.tbody-block').append(   \"<tr>\" +
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='line-height: 51px;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\"+data['block'][i]['height']+\"'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding'>\"+ data['block'][i]['height']+\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a></td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data['block'][i]['time'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data['block'][i]['num'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data['block'][i]['blocksize'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\" +data['block'][i]['blockhash']+ \"' style='float: left;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding' style='white-space:nowrap;'>\"+ data['block'][i]['blockhash'] +\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t    \"</tr>\" );
\t\t\t\t\t}

\t\t\t\t\tif (data['pagination']!=null) {console.log(data['pagination']);data['pagination'] = parseInt(data['pagination']);console.log(data['pagination']);}

\t\t\t\t\tif (data['pagination']>1) {

\t\t\t\t\t\t\$(\"#web_page\").attr('page', '');
\t\t\t\t\t\t\$(\"#web_page\").attr('page', data['pagination']);

\t\t\t\t\t\t\$('#pagination').find('li').empty();
\t\t\t\t\t\t\$(\"#pagination p:first\").text(\"\");
\t\t\t\t\t\t\$(\"#pagination p:first\").text('共'+data['pagination']+\"页 跳到\")

\t\t\t\t\t\tif (data['pagination']>4) {
\t\t\t\t\t\t\t
\t\t\t\t\t\t\tfor(var i = 1; i <= 5; i++){

\t\t\t\t\t\t\t\tif (i==1) {
\t\t\t\t\t\t\t\t\t\$('#pagination').append(\"<li><a href='javascript:void(0);' id='pagination_1' page='1' style='background-color: #ebebeb; color: #777777'>1</a></li>\"); 
\t\t\t\t\t\t\t\t}else{
\t\t\t\t\t\t\t\t\t\$('#pagination').append(\"<li><a href='javascript:void(0);' id='pagination_\"+i+\"' page='\"+i+\"' style='color: #777777'>\"+i+\"</a></li>\"); 
\t\t\t\t\t\t\t\t}

\t\t\t\t\t\t\t}

\t\t\t\t\t\t\t\$('#pagination').append(\"<li><a href='javascript:void(0);' id='pagination_next' page='2' style='color: #777777'>&raquo;</a></li>\"); 
\t\t\t\t\t\t\t
\t\t\t\t\t\t} else {

\t\t\t\t\t\t\tfor(var i = 1; i <= data['pagination']; i++){
\t\t\t\t\t\t\t\tif (i==1) {
\t\t\t\t\t\t\t\t\t\$('#pagination').append(\"<li><a href='javascript:void(0);' id='pagination_1' page='1' class='avtive' style='background-color: #ebebeb; color: #777777'>\"+i+\"</a></li>\"); 
\t\t\t\t\t\t\t\t} else {
\t\t\t\t\t\t\t\t\t\$('#pagination').append(\"<li><a href='javascript:void(0);' id='pagination_\"+i+\"' page=\"+i+\" style='color: #777777'>\"+i+\"</a></li>\"); 
\t\t\t\t\t\t\t\t}
\t\t\t\t\t\t\t}

\t\t\t\t\t\t}

\t\t\t\t\t\t\$('#pagination').show();

\t\t\t\t\t} 

\t\t\t\t} else {

\t\t\t\t\t\$(\"#pagination\").hide();
\t\t\t\t\t\$('.tbody-block').empty();
\t\t\t\t\t\$('#show-more').remove();
\t\t\t\t\t\$('#no-block').show();

\t\t\t\t}

\t\t\t});

\t    });

\t});

\t\$(document).on(\"click\",\"#show-more\",function(){

\t\t\$('#show-more').hide();
\t\t\$('#div-loader').show();

\t\tvar date = \$('#show-more').attr('date');

\t\t\$.post('/more_block', {date:date}, function(data,status){

\t\t\tif (data != null) {

\t\t\t\t\$('#div-loader').hide();

\t\t\t\tvar data = JSON.parse(data);

\t\t\t\tfor (var i = 0; i < data.length; i++) {

\t\t\t\t\t\$('.tbody-block').append(   \"<tr>\" +
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='line-height: 51px;'>\"+
\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\"+data[i]['height']+\"'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding'>\"+ data[i]['height']+\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a></td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data[i]['time'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data[i]['num'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data[i]['blocksize'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+
\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\" +data[i]['blockhash']+ \"' style='float: left;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding' style='white-space:nowrap;'>\"+ data[i]['blockhash'] +\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t    \"</tr>\" );

\t\t\t\t}

\t\t\t}

\t\t});

\t});


\t\$(\"#pagination a\").bind(\"click\", function(event){
\t
\t\tevent.stopImmediatePropagation();

\t\tvar page = \$(this).attr('page');
\t\tvar total_page = \$(\"#web_page\").attr('page');
\t\tvar date = \$('#web_date').attr('date');
\t\tconsole.log(page+total_page+date);
\t\tgetPageInfo(page, total_page, date);
\t\t
\t});


\t\$('#page-input').bind('keydown', function(event){

        if(event.keyCode == \"13\")    
        {

        \tvar page = \$(this).val();
        \tvar total_page = \$(\"#web_page\").attr('page');
        \tvar date = \$('#web_date').attr('date');
         \t
         \tif (page != null) {

         \t\tgetPageInfo(page, total_page, date);

         \t}

        }

    });

    function pageInfo(){

    \tvar page = \$('#page-input').val();
    \tvar total_page = \$(\"#web_page\").attr('page');
    \tvar date = \$('#web_date').attr('date');
     \t
     \tif (page != null) {

     \t\tgetPageInfo(page, total_page, date);

     \t}

    }

    function getPageInfo(page, total_page, date){
    \t
    \t\$.get(\"/all_blocks_pagination\", {page:page, total_page:total_page, date:date}, function (data, status) {
   \t\t\t
       \t\tif (data != null) {

       \t\t\t\$('.tbody-block').empty();

       \t\t\tvar data = JSON.parse(data);

       \t\t\tif (total_page <= 5) {

       \t\t\t\t\$(\"#pagination\").find('li').remove();

       \t\t\t\tif (data['page'] > 1) {
       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_previous' page='\"+  (data['page']-1) +\"' style='color: #777777'>&laquo;</a></li>\");
       \t\t\t\t\tregisterClick(\"#pagination_previous\");
       \t\t\t\t}
\t\t\t\t
       \t\t\t\tfor(var i = 0; i < total_page; i++){

       \t\t\t\t\tif (data['page'] == i) {
       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='background-color: #ebebeb; color: #777777'>\"+ i +\"</a></li>\");
       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
       \t\t\t\t\t} else {
       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='color: #777777'>\"+ i +\"</a></li>\");
       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
       \t\t\t\t\t}  

       \t\t\t\t}
\t\t\t\t\t
\t\t\t\t\tif (data['page'] < total_page) {
       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_next' page='\"+ (data['page']+1) +\"' style='color: #777777'>&raquo;</a></li>\");
       \t\t\t\t\tregisterClick(\"#pagination_next\");
       \t\t\t\t}

       \t\t\t} else if (total_page > 5) {

       \t\t\t\tif (total_page-data['page']>=2 && data['page']>2) {  //中间情况

       \t\t\t\t\t\$(\"#pagination\").find('li').remove();

\t       \t\t\t\tif (data['page'] > 1) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_previous' page='\"+ (data['page']-1) +\"' style='color: #777777'>&laquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_previous\");
\t       \t\t\t\t}
\t\t\t\t\t\t
\t       \t\t\t\tfor(var i = data['page']-2; i <= data['page']+2; i++){

\t       \t\t\t\t\tif (data['page'] == i) {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='background-color: #ebebeb; color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t} else {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t}  

\t       \t\t\t\t}
\t\t\t\t\t\t
\t\t\t\t\t\tif (data['page'] < total_page) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_next' page='\"+ (data['page']+1) +\"' style='color: #777777'>&raquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_next\");
\t       \t\t\t\t}

       \t\t\t\t} else if (data['page']<=2){ //最左情况

       \t\t\t\t\t\$(\"#pagination\").find('li').remove();

\t       \t\t\t\tif (data['page'] > 1) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_previous' page='\"+ (data['page']-1) +\"' style='color: #777777'>&laquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_previous\");
\t       \t\t\t\t}
\t\t\t\t\t
\t       \t\t\t\tfor(var i = 1; i <= 5; i++){

\t       \t\t\t\t\tif (data['page'] == i) {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='background-color: #ebebeb; color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t} else {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t}  

\t       \t\t\t\t}
\t\t\t\t\t\t
\t\t\t\t\t\tif (data['page'] < total_page) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_next' page='\"+ (data['page']+1) +\"' style='color: #777777'>&raquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_next\");
\t       \t\t\t\t}


       \t\t\t\t} else if (total_page-data['page']<2){ //最右情况

       \t\t\t\t\t\$(\"#pagination\").find('li').remove();

\t       \t\t\t\tif (data['page'] > 1) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_previous' page='\"+ (data['page']-1) +\"' style='color: #777777'>&laquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_previous\");
\t       \t\t\t\t}
\t\t\t\t\t
\t       \t\t\t\tfor(var i = total_page-4; i <= total_page; i++){

\t       \t\t\t\t\tif (data['page'] == i) {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='background-color: #ebebeb; color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t} else {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t}  

\t       \t\t\t\t}
\t\t\t\t\t\t
\t\t\t\t\t\tif (data['page'] < total_page) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_next' page='\"+ (data['page']+1) +\"' style='color: #777777'>&raquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_next\");
\t       \t\t\t\t}

       \t\t\t\t}

       \t\t\t}

\t\t\t\tfor (var i = 0; i < data['block'].length; i++) {

\t\t\t\t\t\$('.tbody-block').append(   \"<tr>\" +
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='line-height: 51px;'>\"+
\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\"+data['block'][i]['height']+\"'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding'>\"+ data['block'][i]['height']+\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a></td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data['block'][i]['time'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data['block'][i]['num'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+ data['block'][i]['blocksize'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left; line-height: 51px;'>\"+
\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\" +data['block'][i]['blockhash']+ \"' style='float: left;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding' style='white-space:nowrap;'>\"+ data['block'][i]['blockhash'] +\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t    \"</tr>\" );

\t\t\t\t}

       \t\t}

\t\t});

    }

    function registerClick(id) {

    \t\$(id).unbind('click');

    \t\$(id).bind(\"click\", function(event){
\t\t
\t\t\tevent.stopImmediatePropagation();

\t\t\tvar page = \$(this).attr('page');
\t\t\tvar total_page = \$(\"#web_page\").attr('page');
\t\t\tvar date = \$('#web_date').attr('date');
\t\t\tgetPageInfo(page, total_page, date);
\t\t\t
\t\t});

    }

</script>", "D:\\MgcBrowser/themes/magnachain/pages/all-blocks.htm", "");
    }
}
